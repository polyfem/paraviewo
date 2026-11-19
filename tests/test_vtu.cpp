#include <paraviewo/VTUWriter.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <tinyxml2.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

namespace
{
	struct TemporaryFile
	{
		std::filesystem::path path = std::filesystem::temp_directory_path()
									 / ("paraviewo-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".vtu");
		~TemporaryFile()
		{
			std::error_code error;
			std::filesystem::remove(path, error);
		}
	};

	std::string read_file(const std::filesystem::path &path)
	{
		std::ifstream input(path, std::ios::binary);
		REQUIRE(input.good());
		return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
	}

	struct Block
	{
		std::string type;
		std::string payload;

		template <typename T>
		std::vector<T> values() const
		{
			REQUIRE(payload.size() % sizeof(T) == 0);
			std::vector<T> result(payload.size() / sizeof(T));
			if (!payload.empty())
				std::memcpy(result.data(), payload.data(), payload.size());
			return result;
		}
	};

	// Parse XML metadata separately, then follow each offset into the raw payload.
	std::map<std::string, Block> read_appended(const std::filesystem::path &path, int points, int cells)
	{
		const std::string bytes = read_file(path);
		const std::string marker = "<AppendedData encoding=\"raw\">\n_";
		const size_t marker_pos = bytes.find(marker);
		REQUIRE(marker_pos != std::string::npos);
		const size_t start = marker_pos + marker.size();
		tinyxml2::XMLDocument xml;
		const std::string metadata = bytes.substr(0, marker_pos) + "</VTKFile>";
		REQUIRE(xml.Parse(metadata.c_str()) == tinyxml2::XML_SUCCESS);
		const auto *root = xml.FirstChildElement("VTKFile");
		REQUIRE(root != nullptr);
		CHECK(std::string(root->Attribute("header_type")) == "UInt64");
		CHECK(std::string(root->Attribute("byte_order")) == "LittleEndian");
		const auto *grid = root->FirstChildElement("UnstructuredGrid");
		REQUIRE(grid != nullptr);
		const auto *piece = grid->FirstChildElement("Piece");
		REQUIRE(piece != nullptr);
		CHECK(piece->IntAttribute("NumberOfPoints") == points);
		CHECK(piece->IntAttribute("NumberOfCells") == cells);
		std::map<std::string, Block> result;
		uint64_t next_offset = 0;
		for (const auto *section = piece->FirstChildElement(); section; section = section->NextSiblingElement())
			for (const auto *array = section->FirstChildElement("DataArray"); array; array = array->NextSiblingElement("DataArray"))
			{
				REQUIRE(std::string(array->Attribute("format")) == "appended");
				uint64_t offset = 0;
				REQUIRE(array->QueryUnsigned64Attribute("offset", &offset) == tinyxml2::XML_SUCCESS);
				REQUIRE(offset == next_offset);
				REQUIRE(offset <= bytes.size() - start);
				REQUIRE(sizeof(uint64_t) <= bytes.size() - start - offset);
				uint64_t size = 0;
				std::memcpy(&size, bytes.data() + start + offset, sizeof(size));
				REQUIRE(size <= bytes.size() - start - offset - sizeof(size));
				const std::string name = array->Attribute("Name") ? array->Attribute("Name") : "points";
				REQUIRE(result.count(name) == 0);
				result.emplace(name, Block{array->Attribute("type"), bytes.substr(start + offset + sizeof(size), size)});
				next_offset = offset + sizeof(size) + size;
			}
		CHECK(bytes.substr(start + next_offset) == "\n</AppendedData>\n</VTKFile>\n");
		return result;
	}
} // namespace

TEST_CASE("VTU raw blocks preserve geometry and point and cell fields", "[vtu]")
{
	const int dimension = GENERATE(1, 2, 3);
	TemporaryFile output;
	Eigen::MatrixXd points(4, dimension);
	for (int i = 0; i < points.rows(); ++i)
		for (int d = 0; d < dimension; ++d)
			points(i, d) = i * 3 + d + 1;
	Eigen::MatrixXi cells(2, 3);
	cells << 0, 1, 2, 1, 3, 2;
	Eigen::MatrixXd scalar(4, 1), vector(4, 2), cell_scalar(2, 1), cell_vector(2, 2);
	scalar << 1, 2, 3, 4;
	vector << 1, 2, 3, 4, 5, 6, 7, 8;
	cell_scalar << 9, 10;
	cell_vector << 11, 12, 13, 14;
	paraviewo::VTUWriter writer;
	writer.add_field("scalar", scalar);
	writer.add_field("vector", vector);
	writer.add_cell_field("cell_scalar", cell_scalar);
	writer.add_cell_field("cell_vector", cell_vector);
	REQUIRE(writer.write_mesh(output.path.string(), points, cells, paraviewo::CellType::Triangle));
	const auto blocks = read_appended(output.path, 4, 2);
	REQUIRE(blocks.size() == 8);
	std::vector<double> expected_points;
	for (int i = 0; i < points.rows(); ++i)
		for (int d = 0; d < 3; ++d)
			expected_points.push_back(d < dimension ? points(i, d) : 0);
	CHECK(blocks.at("points").values<double>() == expected_points);
	CHECK(blocks.at("scalar").values<double>() == std::vector<double>{1, 2, 3, 4});
	CHECK(blocks.at("vector").values<double>() == std::vector<double>{1, 2, 0, 3, 4, 0, 5, 6, 0, 7, 8, 0});
	CHECK(blocks.at("cell_scalar").values<double>() == std::vector<double>{9, 10});
	CHECK(blocks.at("cell_vector").values<double>() == std::vector<double>{11, 12, 0, 13, 14, 0});
	CHECK(blocks.at("connectivity").type == "Int32");
	CHECK(blocks.at("connectivity").values<int32_t>() == std::vector<int32_t>{0, 1, 2, 1, 3, 2});
	CHECK(blocks.at("offsets").type == "Int32");
	CHECK(blocks.at("offsets").values<int32_t>() == std::vector<int32_t>{3, 6});
	CHECK(blocks.at("types").type == "UInt8");
	CHECK(blocks.at("types").values<uint8_t>() == std::vector<uint8_t>{5, 5});

	// Reuse the writer and replace the same file with fewer fields and cells.
	const std::vector<paraviewo::CellElement> smaller = {{{0, 1, 2}, paraviewo::CellType::Triangle}};
	REQUIRE(writer.write_mesh(output.path.string(), points, smaller));
	const auto replaced = read_appended(output.path, 4, 1);
	CHECK(replaced.size() == 4);
	CHECK(replaced.at("connectivity").values<int32_t>() == std::vector<int32_t>{0, 1, 2});
}

TEST_CASE("VTU raw blocks preserve mixed cells and empty meshes", "[vtu]")
{
	TemporaryFile output;
	const Eigen::MatrixXd points = Eigen::MatrixXd::Zero(4, 2);
	const std::vector<paraviewo::CellElement> cells = {
		{{0, 1, 2}, paraviewo::CellType::Triangle},
		{{0, 1, 3, 2}, paraviewo::CellType::Quadrilateral}};
	paraviewo::VTUWriter writer;
	REQUIRE(writer.write_mesh(output.path.string(), points, cells));
	const auto blocks = read_appended(output.path, 4, 2);
	CHECK(blocks.at("connectivity").values<int32_t>() == std::vector<int32_t>{0, 1, 2, 0, 1, 3, 2});
	CHECK(blocks.at("offsets").values<int32_t>() == std::vector<int32_t>{3, 7});
	CHECK(blocks.at("types").values<uint8_t>() == std::vector<uint8_t>{5, 9});
	const bool mixed = GENERATE(false, true);
	if (mixed)
		REQUIRE(writer.write_mesh(output.path.string(), Eigen::MatrixXd(0, 3), std::vector<paraviewo::CellElement>{}));
	else
		REQUIRE(writer.write_mesh(output.path.string(), Eigen::MatrixXd(0, 3), Eigen::MatrixXi(0, 4), paraviewo::CellType::Tetrahedron));
	const auto empty = read_appended(output.path, 0, 0);
	REQUIRE(empty.size() == 4);
	for (const auto &[name, block] : empty)
		CHECK(block.payload.empty());
}

TEST_CASE("VTU ASCII output remains valid XML", "[vtu]")
{
	TemporaryFile output;
	Eigen::MatrixXd points(3, 2);
	points << 0, 0, 1, 0, 0, 1;
	Eigen::MatrixXi cells(1, 3);
	cells << 0, 1, 2;
	paraviewo::VTUWriter writer(false);
	REQUIRE(writer.write_mesh(output.path.string(), points, cells, paraviewo::CellType::Triangle));
	tinyxml2::XMLDocument xml;
	REQUIRE(xml.LoadFile(output.path.string().c_str()) == tinyxml2::XML_SUCCESS);
	const auto *piece = xml.FirstChildElement("VTKFile")->FirstChildElement("UnstructuredGrid")->FirstChildElement("Piece");
	const auto *types = piece->FirstChildElement("Cells")->FirstChildElement("DataArray")->NextSiblingElement("DataArray");
	CHECK(std::string(types->Attribute("type")) == "UInt8");
	CHECK(std::string(types->GetText()).find('5') != std::string::npos);
	CHECK(read_file(output.path).find("AppendedData") == std::string::npos);
}
