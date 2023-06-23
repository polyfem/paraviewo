#pragma once

#include "ParaviewWriter.hpp"

#include <h5pp/h5pp.h>

#include <Eigen/Dense>

#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <array>

namespace paraviewo
{

	template <typename T>
	class HDF5VTKDataNode
	{

	public:
		HDF5VTKDataNode(const bool is_point)
			: is_point_(is_point)
		{
		}

		HDF5VTKDataNode(const bool is_point, const std::string &name, const Eigen::MatrixXd &data = Eigen::MatrixXd())
			: is_point_(is_point), name_(name), data_(data)
		{
		}

		// const inline Eigen::MatrixXd &data() { return data_; }

		void initialize(const std::string &name, const Eigen::MatrixXd &data)
		{
			name_ = name;
			data_ = data;
		}

		void write(h5pp::File &file) const
		{
			const std::string key = is_point_ ? "PointData" : "CellData";

			assert(data_.cols() == 1 || data_.cols() == 3);
			if (data_.cols() == 3)
				file.writeDataset(data_, "/VTKHDF/" + key + "/" + name_);
			else
			{
				Eigen::Matrix<T, Eigen::Dynamic, 1> tmp = data_.col(0);
				file.writeDataset(tmp, "/VTKHDF/" + key + "/" + name_);
			}
		}

		inline bool empty() const { return data_.size() <= 0; }

	private:
		const bool is_point_;
		std::string name_;
		Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> data_;
		int n_components_;
	};

	class HDF5VTUWriter : public ParaviewWriter
	{
	public:
		HDF5VTUWriter(bool binary = true);

		bool write_mesh(const std::string &path, const Eigen::MatrixXd &points, const Eigen::MatrixXi &cells) override;
		bool write_mesh(const std::string &path, const Eigen::MatrixXd &points, const std::vector<std::vector<int>> &cells, const bool is_simplicial, const bool has_poly) override;

		void clear() override;

	protected:
		void add_scalar_field(const std::string &name, const Eigen::MatrixXd &data) override;
		void add_vector_field(const std::string &name, const Eigen::MatrixXd &data) override;

		void add_scalar_cell_field(const std::string &name, const Eigen::MatrixXd &data) override;
		void add_vector_cell_field(const std::string &name, const Eigen::MatrixXd &data) override;

	private:
		bool is_volume_;

		std::vector<HDF5VTKDataNode<double>> point_data_;
		std::string current_scalar_point_data_;
		std::string current_vector_point_data_;

		std::vector<HDF5VTKDataNode<double>> cell_data_;
		std::string current_scalar_cell_data_;
		std::string current_vector_cell_data_;

		void write_data(h5pp::File &file);
		void write_header(const int n_vertices, const int n_elements, const std::string &grp, h5pp::File &file);
		void write_points(const Eigen::MatrixXd &points, h5pp::File &file);
		void write_cells(const Eigen::MatrixXi &cells, const std::string &grp, h5pp::File &file);
		void write_cells(const std::vector<std::vector<int>> &cells, const bool is_simplex, const bool is_poly, const std::string &grp, h5pp::File &file);
	};

} // namespace paraviewo
