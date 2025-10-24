#include "VTUWriter.hpp"

namespace paraviewo
{

	VTUWriter::VTUWriter(bool binary)
		: binary_(binary)
	{
	}

	void VTUWriter::write_point_data(std::ostream &os)
	{
		if (current_scalar_point_data_.empty() && current_vector_point_data_.empty())
			return;

		os << "<PointData ";
		if (!current_scalar_point_data_.empty())
			os << "Scalars=\"" << current_scalar_point_data_ << "\" ";
		if (!current_vector_point_data_.empty())
			os << "Vectors=\"" << current_vector_point_data_ << "\" ";
		os << ">\n";

		for (auto it = point_data_.begin(); it != point_data_.end(); ++it)
		{
			it->write(os);
		}

		os << "</PointData>\n";
	}

	void VTUWriter::write_cell_data(std::ostream &os)
	{
		if (current_scalar_cell_data_.empty() && current_vector_cell_data_.empty())
			return;

		os << "<CellData ";
		if (!current_scalar_cell_data_.empty())
			os << "Scalars=\"" << current_scalar_cell_data_ << "\" ";
		if (!current_vector_cell_data_.empty())
			os << "Vectors=\"" << current_vector_cell_data_ << "\" ";
		os << ">\n";

		for (auto it = cell_data_.begin(); it != cell_data_.end(); ++it)
		{
			it->write(os);
		}

		os << "</CellData>\n";
	}

	void VTUWriter::write_header(const int n_vertices, const int n_elements, std::ostream &os)
	{
		os << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" header_type=\"UInt64\">\n";
		os << "<UnstructuredGrid>\n";
		os << "<Piece NumberOfPoints=\"" << n_vertices << "\" NumberOfCells=\"" << n_elements << "\">\n";
	}

	void VTUWriter::write_footer(std::ostream &os)
	{
		os << "</Piece>\n";
		os << "</UnstructuredGrid>\n";
		os << "</VTKFile>\n";
	}

	void VTUWriter::write_points(const Eigen::MatrixXd &points, std::ostream &os)
	{
		os << "<Points>\n";
		if (binary_)
		{

			Eigen::MatrixXd tmp = points.transpose();
			if (tmp.rows() != 3)
			{
				tmp.conservativeResize(3, tmp.cols());
				tmp.row(2).setZero();
			}

			base64Layer base64(os);

			os << "<DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"binary\">\n";
			const uint64_t size = tmp.size() * sizeof(double);
			base64.write(size);

			base64.write(tmp.data(), tmp.size());
			base64.close();
			os << "\n";
		}
		else
		{
			os << "<DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n";

			for (int d = 0; d < points.rows(); ++d)
			{

				for (int i = 0; i < points.cols(); ++i)
				{
					os << points(d, i);
					if (i < points.cols() - 1)
					{
						os << " ";
					}
				}

				if (!(points.cols() == 3))
					os << " 0";

				os << "\n";
			}
		}

		os << "</DataArray>\n";
		os << "</Points>\n";
	}

	void VTUWriter::write_cells(const Eigen::MatrixXi &cells, const CellType ctype, std::ostream &os)
	{
		const int n_cells = cells.rows();
		const int n_cell_vertices = cells.cols();
		os << "<Cells>\n";
		base64Layer base64(os);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (binary_)
		{
			os << "<DataArray type=\"Int64\" Name=\"connectivity\" format=\"binary\" >\n";
			const uint64_t size = cells.size() * sizeof(int64_t);
			base64.write(size);

			Eigen::Matrix<int64_t, Eigen::Dynamic, Eigen::Dynamic> tmp = cells.transpose().template cast<int64_t>();
			base64.write(tmp.data(), tmp.size());
			base64.close();
			os << "\n";
		}
		else
		{
			os << "<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n";

			for (int c = 0; c < n_cells; ++c)
			{
				for (int i = 0; i < n_cell_vertices; ++i)
				{
					const int64_t v_index = cells(c, i);

					os << v_index;
					if (i < n_cell_vertices - 1)
					{
						os << " ";
					}
				}
			}

			os << "\n";
		}

		os << "</DataArray>\n";
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		const int int_tag = paraview_tags::VTKTag(n_cell_vertices, ctype);
		if (binary_)
		{
			os << "<DataArray type=\"Int8\" Name=\"types\" format=\"binary\">\n";
			const uint64_t size = n_cells * sizeof(int8_t);
			base64.write(size);
		}
		else
			os << "<DataArray type=\"Int8\" Name=\"types\" format=\"ascii\">\n";

		for (int i = 0; i < n_cells; ++i)
		{
			const int8_t tag = int_tag;
			if (binary_)
				base64.write(tag);
			else
				os << tag << "\n";
		}
		if (binary_)
		{
			base64.close();
			os << "\n";
		}
		os << "</DataArray>\n";

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (binary_)
		{
			os << "<DataArray type=\"Int64\" Name=\"offsets\" format=\"binary\">\n";
			const uint64_t size = n_cells * sizeof(int64_t);
			base64.write(size);
		}
		else
			os << "<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n";

		int64_t acc = n_cell_vertices;
		for (int i = 0; i < n_cells; ++i)
		{
			if (binary_)
				base64.write(acc);
			else
				os << acc << "\n";
			acc += n_cell_vertices;
		}
		if (binary_)
		{
			base64.close();
			os << "\n";
		}

		os << "</DataArray>\n";
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		os << "</Cells>\n";
	}

	void VTUWriter::write_cells(const std::vector<CellElement> &cells, std::ostream &os)
	{
		const int n_cells = cells.size();
		os << "<Cells>\n";
		base64Layer base64(os);

		int n_cells_indices = 0;
		for (const auto &c : cells)
			n_cells_indices += c.vertices.size();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (binary_)
		{
			os << "<DataArray type=\"Int64\" Name=\"connectivity\" format=\"binary\" >\n";
			const uint64_t size = n_cells_indices * sizeof(int64_t);
			base64.write(size);

			Eigen::Matrix<int64_t, Eigen::Dynamic, 1> tmp(n_cells_indices);
			int index = 0;
			for (const auto &c : cells)
			{
				for (const int i : c.vertices)
					tmp(index++) = i;
			}
			base64.write(tmp.data(), tmp.size());
			base64.close();
			os << "\n";
		}
		else
		{
			os << "<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n";

			for (const auto &c : cells)
			{
				for (size_t i = 0; i < c.vertices.size(); ++i)
				{
					const int64_t v_index = c.vertices[i];

					os << v_index;
					if (i < c.vertices.size() - 1)
					{
						os << " ";
					}
				}
				os << "\n";
			}
		}

		os << "</DataArray>\n";
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (binary_)
		{
			os << "<DataArray type=\"UInt8\" Name=\"types\" format=\"binary\">\n";
			const uint64_t size = n_cells * sizeof(uint8_t);
			base64.write(size);
		}
		else
			os << "<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n";

		for (int i = 0; i < n_cells; ++i)
		{
			const int int_tag = paraview_tags::VTKTag(cells[i].vertices.size(), cells[i].ctype);
			const uint8_t tag = int_tag;

			if (binary_)
				base64.write(tag);
			else
				os << int_tag << "\n";
		}
		if (binary_)
		{
			base64.close();
			os << "\n";
		}
		os << "</DataArray>\n";

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (binary_)
		{
			os << "<DataArray type=\"Int64\" Name=\"offsets\" format=\"binary\">\n";
			const uint64_t size = n_cells * sizeof(int64_t);
			base64.write(size);
		}
		else
			os << "<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n";

		int64_t acc = 0;
		for (int i = 0; i < n_cells; ++i)
		{
			acc += cells[i].vertices.size();
			if (binary_)
				base64.write(acc);
			else
				os << acc << "\n";
		}
		if (binary_)
		{
			base64.close();
			os << "\n";
		}

		os << "</DataArray>\n";
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		os << "</Cells>\n";
	}

	void VTUWriter::clear()
	{
		point_data_.clear();
		cell_data_.clear();
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
		std::ofstream os;
		os.open(path.c_str());
		if (!os.good())
		{
			os.close();
			return false;
		}

		write_header(points.rows(), cells.rows(), os);
		write_points(points, os);
		write_point_data(os);
		write_cell_data(os);
		write_cells(cells, ctype, os);

		write_footer(os);
		os.close();
		clear();
		return true;
	}

	bool VTUWriter::write_mesh(const std::string &path, const Eigen::MatrixXd &points, const std::vector<std::vector<int>> &cells, const CellType ctype)
	{
		Eigen::MatrixXi cells_mat(cells.size(), cells[0].size());
		for (int i = 0; i < cells.size(); ++i)
			for (int j = 0; j < cells[i].size(); ++j)
				cells_mat(i, j) = cells[i][j];
		return write_mesh(path, points, cells_mat, ctype);
	}

	bool VTUWriter::write_mesh(const std::string &path, const Eigen::MatrixXd &points, const std::vector<CellElement> &cells)
	{
		std::ofstream os;
		os.open(path.c_str());
		if (!os.good())
		{
			os.close();
			return false;
		}

		write_header(points.rows(), cells.size(), os);
		write_points(points, os);
		write_point_data(os);
		write_cell_data(os);
		write_cells(cells, os);

		write_footer(os);
		os.close();
		clear();
		return true;
	}
} // namespace paraviewo
