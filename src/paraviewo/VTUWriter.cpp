#include "VTUWriter.hpp"

#include <limits>

namespace paraviewo
{
	namespace
	{
		template <typename T>
		uint64_t appended_block_size(const uint64_t count)
		{
			return sizeof(uint64_t) + count * sizeof(T);
		}

		template <typename T>
		void write_appended_block(std::ostream &os, const T *data, const uint64_t count)
		{
			const uint64_t size = count * sizeof(T);
			os.write(reinterpret_cast<const char *>(&size), sizeof(size));
			os.write(reinterpret_cast<const char *>(data), static_cast<std::streamsize>(size));
		}

		Eigen::MatrixXd padded_points(const Eigen::MatrixXd &points)
		{
			Eigen::MatrixXd tmp = Eigen::MatrixXd::Zero(3, points.rows());
			const Eigen::Index n_dims = points.cols() < 3 ? points.cols() : 3;
			tmp.topRows(n_dims) = points.leftCols(n_dims).transpose();
			return tmp;
		}

		template <typename OffsetT>
		void write_uniform_offsets(std::ostream &os, const int n_cells, const int n_cell_vertices)
		{
			Eigen::Matrix<OffsetT, Eigen::Dynamic, 1> offsets(n_cells);
			OffsetT acc = static_cast<OffsetT>(n_cell_vertices);
			for (int i = 0; i < n_cells; ++i)
			{
				offsets(i) = acc;
				acc += static_cast<OffsetT>(n_cell_vertices);
			}

			write_appended_block(os, offsets.data(), static_cast<uint64_t>(offsets.size()));
		}

		template <typename OffsetT>
		void write_variable_offsets(std::ostream &os, const std::vector<CellElement> &cells)
		{
			Eigen::Matrix<OffsetT, Eigen::Dynamic, 1> offsets(cells.size());
			OffsetT acc = 0;
			for (int i = 0; i < static_cast<int>(cells.size()); ++i)
			{
				acc += static_cast<OffsetT>(cells[i].vertices.size());
				offsets(i) = acc;
			}

			write_appended_block(os, offsets.data(), static_cast<uint64_t>(offsets.size()));
		}
	} // namespace

	VTUWriter::VTUWriter(bool binary)
		: binary_(binary)
	{
	}

	void VTUWriter::write_point_data(std::ostream &os, uint64_t &offset)
	{
		if (current_scalar_point_data_.empty() && current_vector_point_data_.empty())
			return;

		os << "<PointData ";
		if (!current_scalar_point_data_.empty())
			os << "Scalars=\"" << current_scalar_point_data_ << "\" ";
		if (!current_vector_point_data_.empty())
			os << "Vectors=\"" << current_vector_point_data_ << "\" ";
		os << ">\n";

		for (const auto &node : point_data_)
		{
			node.write(os, offset);
		}

		os << "</PointData>\n";
	}

	void VTUWriter::write_point_data_appended(std::ostream &os) const
	{
		for (const auto &node : point_data_)
		{
			node.write_appended(os);
		}
	}

	void VTUWriter::write_cell_data(std::ostream &os, uint64_t &offset)
	{
		if (current_scalar_cell_data_.empty() && current_vector_cell_data_.empty())
			return;

		os << "<CellData ";
		if (!current_scalar_cell_data_.empty())
			os << "Scalars=\"" << current_scalar_cell_data_ << "\" ";
		if (!current_vector_cell_data_.empty())
			os << "Vectors=\"" << current_vector_cell_data_ << "\" ";
		os << ">\n";

		for (const auto &node : cell_data_)
		{
			node.write(os, offset);
		}

		os << "</CellData>\n";
	}

	void VTUWriter::write_cell_data_appended(std::ostream &os) const
	{
		for (const auto &node : cell_data_)
		{
			node.write_appended(os);
		}
	}

	void VTUWriter::write_header(const int n_vertices, const int n_elements, std::ostream &os)
	{
		os << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n";
		os << "<UnstructuredGrid>\n";
		os << "<Piece NumberOfPoints=\"" << n_vertices << "\" NumberOfCells=\"" << n_elements << "\">\n";
	}

	void VTUWriter::write_footer(std::ostream &os)
	{
		os << "</Piece>\n";
		os << "</UnstructuredGrid>\n";
	}

	void VTUWriter::write_appended_data_header(std::ostream &os)
	{
		os << "<AppendedData encoding=\"raw\">\n_";
	}

	void VTUWriter::write_appended_data_footer(std::ostream &os)
	{
		os << "\n</AppendedData>\n";
	}

	void VTUWriter::write_file_footer(std::ostream &os)
	{
		os << "</VTKFile>\n";
	}

	void VTUWriter::write_points(const Eigen::MatrixXd &points, std::ostream &os, uint64_t &offset)
	{
		os << "<Points>\n";
		if (binary_)
		{
			os << "<DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"appended\" offset=\"" << offset << "\"/>\n";
			offset += appended_block_size<double>(static_cast<uint64_t>(points.rows()) * 3);
		}
		else
		{
			os << "<DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">\n";
			const int n_dims = points.cols() < 3 ? points.cols() : 3;

			for (int i = 0; i < points.rows(); ++i)
			{
				for (int d = 0; d < n_dims; ++d)
				{
					if (d > 0)
					{
						os << " ";
					}
					os << points(i, d);
				}

				for (int d = n_dims; d < 3; ++d)
				{
					if (d > 0 || n_dims > 0)
					{
						os << " ";
					}
					os << 0;
				}

				os << "\n";
			}

			os << "</DataArray>\n";
		}

		os << "</Points>\n";
	}

	void VTUWriter::write_points_appended(const Eigen::MatrixXd &points, std::ostream &os) const
	{
		const Eigen::MatrixXd tmp = padded_points(points);
		write_appended_block(os, tmp.data(), static_cast<uint64_t>(tmp.size()));
	}

	void VTUWriter::write_cells(const Eigen::MatrixXi &cells, const CellType ctype, std::ostream &os, uint64_t &offset)
	{
		const int n_cells = cells.rows();
		const int n_cell_vertices = cells.cols();
		const uint64_t n_connectivity_ids = static_cast<uint64_t>(cells.size());
		const bool use_32_bit_offsets = n_connectivity_ids <= static_cast<uint64_t>(std::numeric_limits<int32_t>::max());
		const int int_tag = paraview_tags::VTKTag(n_cell_vertices, ctype);

		os << "<Cells>\n";

		if (binary_)
		{
			os << "<DataArray type=\"Int32\" Name=\"connectivity\" format=\"appended\" offset=\"" << offset << "\"/>\n";
			offset += appended_block_size<int32_t>(n_connectivity_ids);
		}
		else
		{
			os << "<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";

			for (int c = 0; c < n_cells; ++c)
			{
				for (int i = 0; i < n_cell_vertices; ++i)
				{
					os << static_cast<int32_t>(cells(c, i));
					if (i < n_cell_vertices - 1)
					{
						os << " ";
					}
				}
				os << "\n";
			}

			os << "</DataArray>\n";
		}

		if (binary_)
		{
			os << "<DataArray type=\"UInt8\" Name=\"types\" format=\"appended\" offset=\"" << offset << "\"/>\n";
			offset += appended_block_size<uint8_t>(n_cells);
		}
		else
		{
			os << "<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n";
			for (int i = 0; i < n_cells; ++i)
			{
				os << int_tag << "\n";
			}
			os << "</DataArray>\n";
		}

		if (binary_)
		{
			os << "<DataArray type=\"" << (use_32_bit_offsets ? "Int32" : "Int64") << "\" Name=\"offsets\" format=\"appended\" offset=\"" << offset << "\"/>\n";
			offset += use_32_bit_offsets ? appended_block_size<int32_t>(n_cells) : appended_block_size<int64_t>(n_cells);
		}
		else
		{
			os << "<DataArray type=\"" << (use_32_bit_offsets ? "Int32" : "Int64") << "\" Name=\"offsets\" format=\"ascii\">\n";

			int64_t acc = n_cell_vertices;
			for (int i = 0; i < n_cells; ++i)
			{
				os << acc << "\n";
				acc += n_cell_vertices;
			}

			os << "</DataArray>\n";
		}

		os << "</Cells>\n";
	}

	void VTUWriter::write_cells_appended(const Eigen::MatrixXi &cells, const CellType ctype, std::ostream &os) const
	{
		const int n_cells = cells.rows();
		const int n_cell_vertices = cells.cols();
		const uint64_t n_connectivity_ids = static_cast<uint64_t>(cells.size());
		const bool use_32_bit_offsets = n_connectivity_ids <= static_cast<uint64_t>(std::numeric_limits<int32_t>::max());
		const uint8_t type = static_cast<uint8_t>(paraview_tags::VTKTag(n_cell_vertices, ctype));

		Eigen::Matrix<int32_t, Eigen::Dynamic, Eigen::Dynamic> connectivity = cells.transpose().template cast<int32_t>();
		write_appended_block(os, connectivity.data(), static_cast<uint64_t>(connectivity.size()));

		Eigen::Matrix<uint8_t, Eigen::Dynamic, 1> types(n_cells);
		types.setConstant(type);
		write_appended_block(os, types.data(), static_cast<uint64_t>(types.size()));

		if (use_32_bit_offsets)
			write_uniform_offsets<int32_t>(os, n_cells, n_cell_vertices);
		else
			write_uniform_offsets<int64_t>(os, n_cells, n_cell_vertices);
	}

	void VTUWriter::write_cells(const std::vector<CellElement> &cells, std::ostream &os, uint64_t &offset)
	{
		const int n_cells = static_cast<int>(cells.size());
		uint64_t n_connectivity_ids = 0;
		for (const auto &cell : cells)
		{
			n_connectivity_ids += static_cast<uint64_t>(cell.vertices.size());
		}
		const bool use_32_bit_offsets = n_connectivity_ids <= static_cast<uint64_t>(std::numeric_limits<int32_t>::max());

		os << "<Cells>\n";

		if (binary_)
		{
			os << "<DataArray type=\"Int32\" Name=\"connectivity\" format=\"appended\" offset=\"" << offset << "\"/>\n";
			offset += appended_block_size<int32_t>(n_connectivity_ids);
		}
		else
		{
			os << "<DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";

			for (const auto &cell : cells)
			{
				for (size_t i = 0; i < cell.vertices.size(); ++i)
				{
					os << static_cast<int32_t>(cell.vertices[i]);
					if (i + 1 < cell.vertices.size())
					{
						os << " ";
					}
				}
				os << "\n";
			}

			os << "</DataArray>\n";
		}

		if (binary_)
		{
			os << "<DataArray type=\"UInt8\" Name=\"types\" format=\"appended\" offset=\"" << offset << "\"/>\n";
			offset += appended_block_size<uint8_t>(n_cells);
		}
		else
		{
			os << "<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n";
			for (const auto &cell : cells)
			{
				os << paraview_tags::VTKTag(cell.vertices.size(), cell.ctype) << "\n";
			}
			os << "</DataArray>\n";
		}

		if (binary_)
		{
			os << "<DataArray type=\"" << (use_32_bit_offsets ? "Int32" : "Int64") << "\" Name=\"offsets\" format=\"appended\" offset=\"" << offset << "\"/>\n";
			offset += use_32_bit_offsets ? appended_block_size<int32_t>(n_cells) : appended_block_size<int64_t>(n_cells);
		}
		else
		{
			os << "<DataArray type=\"" << (use_32_bit_offsets ? "Int32" : "Int64") << "\" Name=\"offsets\" format=\"ascii\">\n";

			int64_t acc = 0;
			for (const auto &cell : cells)
			{
				acc += cell.vertices.size();
				os << acc << "\n";
			}

			os << "</DataArray>\n";
		}

		os << "</Cells>\n";
	}

	void VTUWriter::write_cells_appended(const std::vector<CellElement> &cells, std::ostream &os) const
	{
		const int n_cells = static_cast<int>(cells.size());
		uint64_t n_connectivity_ids = 0;
		for (const auto &cell : cells)
		{
			n_connectivity_ids += static_cast<uint64_t>(cell.vertices.size());
		}
		const bool use_32_bit_offsets = n_connectivity_ids <= static_cast<uint64_t>(std::numeric_limits<int32_t>::max());

		Eigen::Matrix<int32_t, Eigen::Dynamic, 1> connectivity(n_connectivity_ids);
		int index = 0;
		for (const auto &cell : cells)
		{
			for (const int vertex : cell.vertices)
			{
				connectivity(index++) = static_cast<int32_t>(vertex);
			}
		}
		write_appended_block(os, connectivity.data(), static_cast<uint64_t>(connectivity.size()));

		Eigen::Matrix<uint8_t, Eigen::Dynamic, 1> types(n_cells);
		for (int i = 0; i < n_cells; ++i)
		{
			types(i) = static_cast<uint8_t>(paraview_tags::VTKTag(cells[i].vertices.size(), cells[i].ctype));
		}
		write_appended_block(os, types.data(), static_cast<uint64_t>(types.size()));

		if (use_32_bit_offsets)
			write_variable_offsets<int32_t>(os, cells);
		else
			write_variable_offsets<int64_t>(os, cells);
	}

	void VTUWriter::clear()
	{
		point_data_.clear();
		current_scalar_point_data_.clear();
		current_vector_point_data_.clear();
		cell_data_.clear();
		current_scalar_cell_data_.clear();
		current_vector_cell_data_.clear();
	}

	void VTUWriter::add_scalar_field(const std::string &name, const Eigen::MatrixXd &data)
	{
		point_data_.push_back(VTKDataNode<double>(binary_));
		point_data_.back().initialize(name, "Float64", data);
		current_scalar_point_data_ = name;
	}

	void VTUWriter::add_vector_field(const std::string &name, const Eigen::MatrixXd &data)
	{
		point_data_.push_back(VTKDataNode<double>(binary_));

		Eigen::MatrixXd tmp = data;

		if (data.cols() == 2)
		{
			tmp.conservativeResize(tmp.rows(), 3);
			tmp.col(2).setZero();
		}

		point_data_.back().initialize(name, "Float64", tmp, tmp.cols());
		current_vector_point_data_ = name;
	}

	void VTUWriter::add_scalar_cell_field(const std::string &name, const Eigen::MatrixXd &data)
	{
		cell_data_.push_back(VTKDataNode<double>(binary_));
		cell_data_.back().initialize(name, "Float64", data);
		current_scalar_cell_data_ = name;
	}

	void VTUWriter::add_vector_cell_field(const std::string &name, const Eigen::MatrixXd &data)
	{
		cell_data_.push_back(VTKDataNode<double>(binary_));

		Eigen::MatrixXd tmp = data;

		if (data.cols() == 2)
		{
			tmp.conservativeResize(tmp.rows(), 3);
			tmp.col(2).setZero();
		}

		cell_data_.back().initialize(name, "Float64", tmp, tmp.cols());
		current_vector_cell_data_ = name;
	}

	bool VTUWriter::write_mesh(const std::string &path, const Eigen::MatrixXd &points, const Eigen::MatrixXi &cells, const CellType ctype)
	{
		std::vector<char> io_buffer(1 << 20);
		std::ofstream os;
		os.open(path.c_str(), std::ios::binary);
		if (!os.good())
		{
			os.close();
			return false;
		}
		os.rdbuf()->pubsetbuf(io_buffer.data(), static_cast<std::streamsize>(io_buffer.size()));

		uint64_t offset = 0;

		write_header(points.rows(), cells.rows(), os);
		write_points(points, os, offset);
		write_point_data(os, offset);
		write_cell_data(os, offset);
		write_cells(cells, ctype, os, offset);

		write_footer(os);
		if (binary_)
		{
			write_appended_data_header(os);
			write_points_appended(points, os);
			write_point_data_appended(os);
			write_cell_data_appended(os);
			write_cells_appended(cells, ctype, os);
			write_appended_data_footer(os);
		}
		write_file_footer(os);
		os.close();
		clear();
		return true;
	}

	bool VTUWriter::write_mesh(const std::string &path, const Eigen::MatrixXd &points, const std::vector<CellElement> &cells)
	{
		std::vector<char> io_buffer(1 << 20);
		std::ofstream os;
		os.open(path.c_str(), std::ios::binary);
		if (!os.good())
		{
			os.close();
			return false;
		}
		os.rdbuf()->pubsetbuf(io_buffer.data(), static_cast<std::streamsize>(io_buffer.size()));

		uint64_t offset = 0;

		write_header(points.rows(), cells.size(), os);
		write_points(points, os, offset);
		write_point_data(os, offset);
		write_cell_data(os, offset);
		write_cells(cells, os, offset);

		write_footer(os);
		if (binary_)
		{
			write_appended_data_header(os);
			write_points_appended(points, os);
			write_point_data_appended(os);
			write_cell_data_appended(os);
			write_cells_appended(cells, os);
			write_appended_data_footer(os);
		}
		write_file_footer(os);
		os.close();
		clear();
		return true;
	}
} // namespace paraviewo
