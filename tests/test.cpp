////////////////////////////////////////////////////////////////////////////////
#include <paraviewo/VTUWriter.hpp>
#include <paraviewo/HDF5VTUWriter.hpp>
#include <paraviewo/PVDWriter.hpp>

#include <Eigen/Dense>

#include <catch2/catch_all.hpp>
////////////////////////////////////////////////////////////////////////////////

using namespace paraviewo;

void run_test(ParaviewWriter &writer, const std::string &name)
{
	Eigen::MatrixXd pts(25, 3);
	pts << 0, 0, 0, 0.25, 0, 0, 0, 0.25, 0, 0.25, 0.25, 0,
		0.5, 0, 0, 0.5, 0.25, 0, 0.75, 0, 0, 0.75, 0.25, 0,
		1, 0, 0, 1, 0.25, 0, 0, 0.5, 0, 0.25, 0.5, 0,
		0.5, 0.5, 0, 0.75, 0.5, 0, 1, 0.5, 0, 0, 0.75, 0,
		0.25, 0.75, 0, 0.5, 0.75, 0, 0.75, 0.75, 0, 1, 0.75, 0,
		0, 1, 0, 0.25, 1, 0, 0.5, 1, 0, 0.75, 1, 0,
		1, 1, 0;
	pts = pts.leftCols(2).eval();

	Eigen::MatrixXd v(25, 1);
	v.setRandom();

	Eigen::MatrixXi tris(2, 3);
	tris << 0, 1, 2,
		4, 5, 6;

	Eigen::MatrixXd v_cell(2, 1);
	v_cell.setRandom();

	writer.add_field("test", v);
	writer.add_cell_field("ctest", v_cell);
	writer.write_mesh(name, pts, tris, CellType::Triangle);
}

void run_test_vecvec_vtu(VTUWriter &writer, const std::string &name)
{
	Eigen::MatrixXd pts(25, 3);
	pts << 0, 0, 0, 0.25, 0, 0, 0, 0.25, 0, 0.25, 0.25, 0,
		0.5, 0, 0, 0.5, 0.25, 0, 0.75, 0, 0, 0.75, 0.25, 0,
		1, 0, 0, 1, 0.25, 0, 0, 0.5, 0, 0.25, 0.5, 0,
		0.5, 0.5, 0, 0.75, 0.5, 0, 1, 0.5, 0, 0, 0.75, 0,
		0.25, 0.75, 0, 0.5, 0.75, 0, 0.75, 0.75, 0, 1, 0.75, 0,
		0, 1, 0, 0.25, 1, 0, 0.5, 1, 0, 0.75, 1, 0,
		1, 1, 0;
	pts = pts.leftCols(2).eval();

	Eigen::MatrixXd v(25, 1);
	v.setRandom();

	std::vector<std::vector<int>> cells;
	cells.resize(2);
	for (int i=0; i<3; i++)
		cells[0].push_back(i);
	for (int i=0; i<3; i++)
		cells[1].push_back(i+3);	

	Eigen::MatrixXi tris(2, 3);
	tris << 0, 1, 2,
		4, 5, 6;

	Eigen::MatrixXd v_cell(2, 1);
	v_cell.setRandom();

	writer.add_field("test", v);
	writer.add_cell_field("ctest", v_cell);
	writer.write_mesh(name, pts, cells, CellType::Triangle);
}

void run_test_vecvec_hdf5(HDF5VTUWriter &writer, const std::string &name)
{
	Eigen::MatrixXd pts(25, 3);
	pts << 0, 0, 0, 0.25, 0, 0, 0, 0.25, 0, 0.25, 0.25, 0,
		0.5, 0, 0, 0.5, 0.25, 0, 0.75, 0, 0, 0.75, 0.25, 0,
		1, 0, 0, 1, 0.25, 0, 0, 0.5, 0, 0.25, 0.5, 0,
		0.5, 0.5, 0, 0.75, 0.5, 0, 1, 0.5, 0, 0, 0.75, 0,
		0.25, 0.75, 0, 0.5, 0.75, 0, 0.75, 0.75, 0, 1, 0.75, 0,
		0, 1, 0, 0.25, 1, 0, 0.5, 1, 0, 0.75, 1, 0,
		1, 1, 0;
	pts = pts.leftCols(2).eval();

	Eigen::MatrixXd v(25, 1);
	v.setRandom();

	std::vector<std::vector<int>> cells;
	cells.resize(2);
	for (int i=0; i<3; i++)
		cells[0].push_back(i);
	for (int i=0; i<3; i++)
		cells[1].push_back(i+3);	

	Eigen::MatrixXi tris(2, 3);
	tris << 0, 1, 2,
		4, 5, 6;

	Eigen::MatrixXd v_cell(2, 1);
	v_cell.setRandom();

	writer.add_field("test", v);
	writer.add_cell_field("ctest", v_cell);
	writer.write_mesh(name, pts, cells, CellType::Triangle);
}

void run_test_prism_quad(ParaviewWriter &writer, const std::string &name)
{
	Eigen::MatrixXd pts(30, 3);
	// https://examples.vtk.org/site/VTKBook/05Chapter5/?utm_source=chatgpt.com
	pts << 0, 0, 0,  0.25, 0, 0,  0, 0.25, 0, 
			0, 0, 1,  0.25, 0, 1,  0, 0.25, 1, 
			0.125,0,0.05,    0.125,0.125,0.05,   0,0.125,0.05,
			0.125,0,1.05,  0.125,0.125,1.05,   0,0.125,1.05,
			0,0.05,0.5, 0.25,0.05,0.5,  0,0.3,0.5,

			0.5, 0, 0, 0.75, 0, 0,  0.5, 0.25, 0, 
			0.5, 0, 1, 0.75, 0, 1,  0.5, 0.25, 1,
			0.625,0,0,   0.625,0.125,0,  0.5,0.125,0, 
			0.625,0,1,   0.625,0.125,1,  0.5,0.125,1,
			0.5,0,0.5, 0.75,0,0.5,  0.5,0.25,0.5;

	Eigen::MatrixXd v(30, 1);
	v.setRandom();

	std::vector<CellElement> cells;
	cells.resize(2);
	for (int i=0; i<15; i++)
		cells[0].vertices.push_back(i);
	for (int i=0; i<15; i++)
		cells[1].vertices.push_back(i+15);	
	cells[0].ctype = CellType::Wedge;
	cells[1].ctype = CellType::Wedge;

	Eigen::MatrixXd v_cell(2, 1);
	v_cell.setRandom();

	writer.add_field("test", v);
	writer.add_cell_field("ctest", v_cell);
	writer.write_mesh(name, pts, cells);
}

void run_test_mixed(ParaviewWriter &writer, const std::string &name)
{
	Eigen::MatrixXd pts(28, 3);
	// https://examples.vtk.org/site/VTKBook/05Chapter5/?utm_source=chatgpt.com
	pts << 0, 0, 0,  0.25, 0, 0,  0, 0.25, 0, 
			0, 0, 1,  0.25, 0, 1,  0, 0.25, 1, 
			0.125,0,0.05,    0.125,0.125,0.05,   0,0.125,0.05,
			0.125,0,1.05,  0.125,0.125,1.05,   0,0.125,1.05,
			0,0.05,0.5, 0.25,0.05,0.5,  0,0.3,0.5,

			0.5, 0, 0,  0.75, 0, 0, 0.75, 0.25, 0,  0.5, 0.25, 0, 0.625, 0.125, 0.25,
			0.625,0,0.02, 0.75,0.125,0.02,  0.625,0.25,0.02, 0.5,0.125,0.02,
			0.5625,0.0825,0.125, 0.6875,0.0825,0.125, 0.6875,0.2075,0.125, 0.5625,0.2075,0.125;

	Eigen::MatrixXd v(28, 1);
	v.setRandom();

	std::vector<CellElement> cells;
	cells.resize(2);
	for (int i=0; i<15; i++)
		cells[0].vertices.push_back(i);
	for (int i=0; i<13; i++)
		cells[1].vertices.push_back(i+15);	
	cells[0].ctype = CellType::Wedge;
	cells[1].ctype = CellType::Pyramid;

	Eigen::MatrixXd v_cell(2, 1);
	v_cell.setRandom();

	writer.add_field("test", v);
	writer.add_cell_field("ctest", v_cell);
	writer.write_mesh(name, pts, cells);
}

template <typename T>
void save_sequence(const std::string &extension)
{
	auto name_func = [&extension](int i) {
		return "test_" + std::to_string(i) + "." + extension;
	};

	for (int i = 0; i < 10; ++i)
	{
		T writer;
		std::string name = name_func(i);
		run_test(writer, name);
	}

	PVDWriter::save_pvd("sim_" + extension + ".pvd", name_func, 9, 0, 0.1, 1);
}

TEST_CASE("vtu_writer", "[utils]")
{
	VTUWriter writer;
	run_test(writer, "test.vtu");
}

TEST_CASE("hdf5_writer", "[utils]")
{
	HDF5VTUWriter writer;
	run_test(writer, "test.hdf");
}

TEST_CASE("vtu_sequence", "[utils]")
{
	save_sequence<VTUWriter>("vtu");
}

TEST_CASE("hdf5_sequence", "[utils]")
{
	save_sequence<HDF5VTUWriter>("hdf");
}

TEST_CASE("vtu_writer_prism_quad", "[utils]")
{
	VTUWriter writer;
	run_test_prism_quad(writer, "test_prism_quad.vtu");
}

TEST_CASE("vtu_writer_mixed", "[utils]")
{
	VTUWriter writer;
	run_test_mixed(writer, "test_mixed.vtu");
}

TEST_CASE("vtu_writer_vecvec_vtu", "[utils]")
{
	VTUWriter writer;
	run_test_vecvec_vtu(writer, "test_vecvec.vtu");
}

TEST_CASE("vtu_writer_vecvec_hdf5", "[utils]")
{
	HDF5VTUWriter writer;
	run_test_vecvec_hdf5(writer, "test_vecvec.vtu");
}