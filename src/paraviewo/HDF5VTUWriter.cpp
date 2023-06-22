#include "HDF5VTUWriter.hpp"

namespace paraviewo
{

	HDF5VTUWriter::HDF5VTUWriter(bool binary)
	{
	}

	void HDF5VTUWriter::write_data(h5pp::File &file)
	{
		if (!current_scalar_point_data_.empty() || !current_vector_point_data_.empty())
		{
			for (auto it = point_data_.begin(); it != point_data_.end(); ++it)
			{
				it->write(file);
			}
		}

		if (!current_scalar_cell_data_.empty() || !current_vector_cell_data_.empty())
		{
			for (auto it = cell_data_.begin(); it != cell_data_.end(); ++it)
			{
				it->write(file);
			}
		}
	}

	void HDF5VTUWriter::write_header(const int n_vertices, const int n_elements, const std::string &grp, h5pp::File &file)
	{
		std::string tmp = "UnstructuredGrid";
		file.writeAttribute(std::array<int64_t, 2>{{1, 0}}, grp, "Version");
		file.writeAttribute(tmp, grp, "Type", 16);

		file.writeDataset(std::array<int64_t, 1>{{n_vertices}}, grp + "/NumberOfPoints", H5D_CONTIGUOUS);
		file.writeDataset(std::array<int64_t, 1>{{n_elements}}, grp + "/NumberOfCells", H5D_CONTIGUOUS);
	}

	void HDF5VTUWriter::write_points(const Eigen::MatrixXd &points, h5pp::File &file)
	{
		Eigen::MatrixXd tmp(points.rows(), 3);

		for (int d = 0; d < points.rows(); ++d)
		{
			for (int i = 0; i < points.cols(); ++i)
			{
				tmp(d, i) = points(d, i);
			}

			if (!is_volume_)
				tmp(d, 2) = 0;
		}

		file.writeDataset(tmp, "/VTKHDF/Points", H5D_CONTIGUOUS);
	}

	void HDF5VTUWriter::write_cells(const Eigen::MatrixXi &cells, const std::string &grp, h5pp::File &file)
	{
		const int n_cells = cells.rows();
		const int n_cell_vertices = cells.cols();
		int index;

		file.writeDataset(std::array<int64_t, 1>{{n_cells * n_cell_vertices}}, grp + "/NumberOfConnectivityIds", H5D_CONTIGUOUS);
		Eigen::Matrix<int64_t, Eigen::Dynamic, 1> connectivity_array(n_cells * n_cell_vertices);
		index = 0;

		for (int c = 0; c < n_cells; ++c)
		{
			for (int i = 0; i < n_cell_vertices; ++i)
			{
				const int64_t v_index = cells(c, i);
				connectivity_array[index++] = v_index;
			}
		}

		assert(index == n_cells * n_cell_vertices);
		assert(connectivity_array.size() == n_cells * n_cell_vertices);

		file.writeDataset(connectivity_array, "/VTKHDF/Connectivity", H5D_CONTIGUOUS);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		const uint8_t int_tag = is_volume_ ? paraview_tags::VTKTagVolume(n_cell_vertices, true, false) : paraview_tags::VTKTagPlanar(n_cell_vertices, true, false);
		Eigen::Matrix<uint8_t, Eigen::Dynamic, 1> type_array(n_cells);
		type_array.setConstant(int_tag);
		file.writeDataset(type_array, "/VTKHDF/Types", H5D_CONTIGUOUS);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		Eigen::Matrix<int64_t, Eigen::Dynamic, 1> offset_array(n_cells + 1);
		index = 0;

		int64_t acc = n_cell_vertices;
		offset_array[index++] = 0;
		for (int i = 0; i < n_cells; ++i)
		{
			offset_array[index++] = acc;
			acc += n_cell_vertices;
		}

		assert(index == n_cells + 1);
		assert(offset_array.size() == n_cells + 1);

		file.writeDataset(offset_array, "/VTKHDF/Offsets", H5D_CONTIGUOUS);
	}

	void HDF5VTUWriter::write_cells(const std::vector<std::vector<int>> &cells, const bool is_simplex, const bool is_poly, const std::string &grp, h5pp::File &file)
	{
		const int n_cells = cells.size();
		int index;

		int n_cells_indices = 0;
		for (const auto &c : cells)
		{
			n_cells_indices += c.size();
		}
		file.writeDataset(std::array<int64_t, 1>{{n_cells_indices}}, grp + "/NumberOfConnectivityIds", H5D_CONTIGUOUS);

		Eigen::Matrix<int64_t, Eigen::Dynamic, 1> connectivity_array(n_cells_indices);
		index = 0;
		for (const auto &c : cells)
		{
			for (const int i : c)
				connectivity_array[index++] = i;
		}

		assert(index == n_cells_indices);
		assert(connectivity_array.size() == n_cells_indices);

		file.writeDataset(connectivity_array, "/VTKHDF/Connectivity", H5D_CONTIGUOUS);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		Eigen::Matrix<uint8_t, Eigen::Dynamic, 1> type_array(n_cells);
		index = 0;

		for (int i = 0; i < n_cells; ++i)
		{
			const int int_tag = is_volume_ ? paraview_tags::VTKTagVolume(cells[i].size(), is_simplex, is_poly) : paraview_tags::VTKTagPlanar(cells[i].size(), is_simplex, is_poly);
			const uint8_t tag = int_tag;
			type_array[index++] = tag;
		}

		assert(index == n_cells);
		assert(type_array.size() == n_cells);

		file.writeDataset(type_array, "/VTKHDF/Types", H5D_CONTIGUOUS);

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		Eigen::Matrix<int64_t, Eigen::Dynamic, 1> offset_array(n_cells + 1);
		index = 0;

		int64_t acc = 0;
		offset_array[index++] = acc;
		for (int i = 0; i < n_cells; ++i)
		{
			acc += cells[i].size();
			offset_array[index++] = acc;
		}

		assert(index == n_cells + 1);
		assert(offset_array.size() == n_cells + 1);

		file.writeDataset(offset_array, "/VTKHDF/Offsets", H5D_CONTIGUOUS);
	}

	void HDF5VTUWriter::clear()
	{
		point_data_.clear();
		cell_data_.clear();
	}

	void HDF5VTUWriter::add_scalar_field(const std::string &name, const Eigen::MatrixXd &data)
	{
		point_data_.push_back(HDF5VTKDataNode<double>(true));
		point_data_.back().initialize(name, data);
		current_scalar_point_data_ = name;
	}

	void HDF5VTUWriter::add_vector_field(const std::string &name, const Eigen::MatrixXd &data)
	{
		point_data_.push_back(HDF5VTKDataNode<double>(true));

		Eigen::MatrixXd tmp = data;

		if (data.cols() == 2)
		{
			tmp.conservativeResize(tmp.rows(), 3);
			tmp.col(2).setZero();
		}

		point_data_.back().initialize(name, tmp);
		current_vector_point_data_ = name;
	}

	void HDF5VTUWriter::add_scalar_cell_field(const std::string &name, const Eigen::MatrixXd &data)
	{
		cell_data_.push_back(HDF5VTKDataNode<double>(false));
		cell_data_.back().initialize(name, data);
		current_scalar_cell_data_ = name;
	}

	void HDF5VTUWriter::add_vector_cell_field(const std::string &name, const Eigen::MatrixXd &data)
	{
		cell_data_.push_back(HDF5VTKDataNode<double>(false));

		Eigen::MatrixXd tmp = data;

		if (data.cols() == 2)
		{
			tmp.conservativeResize(tmp.rows(), 3);
			tmp.col(2).setZero();
		}

		cell_data_.back().initialize(name, tmp);
		current_vector_cell_data_ = name;
	}

	bool HDF5VTUWriter::write_mesh(const std::string &path, const Eigen::MatrixXd &points, const Eigen::MatrixXi &cells)
	{
		is_volume_ = points.cols() == 3;

		h5pp::File file(path, h5pp::FileAccess::REPLACE);
		file.setCompressionLevel(0);
		file.createGroup("VTKHDF");

		write_header(points.rows(), cells.rows(), "VTKHDF", file);
		write_points(points, file);
		write_data(file);
		write_cells(cells, "VTKHDF", file);

		file.flush();
		clear();
		return true;
	}

	bool HDF5VTUWriter::write_mesh(const std::string &path, const Eigen::MatrixXd &points, const std::vector<std::vector<int>> &cells, const bool is_simplicial, const bool has_poly)
	{
		is_volume_ = points.cols() == 3;

		h5pp::File file(path, h5pp::FileAccess::REPLACE);
		file.setCompressionLevel(0);
		file.createGroup("VTKHDF");

		write_header(points.rows(), cells.size(), "VTKHDF", file);
		write_points(points, file);
		write_data(file);
		write_cells(cells, is_simplicial, has_poly, "VTKHDF", file);

		file.flush();
		clear();
		return true;
	}

} // namespace paraviewo
