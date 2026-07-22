#pragma once

#include "ParaviewWriter.hpp"

#include <Eigen/Dense>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace paraviewo
{

	template <typename T>
	class VTKDataNode
	{

	public:
		VTKDataNode(bool binary)
			: binary_(binary)
		{
		}

		VTKDataNode(const std::string &name, const bool binary, const std::string &numeric_type, const Eigen::MatrixXd &data = Eigen::MatrixXd(), const int n_components = 1)
			: name_(name), binary_(binary), numeric_type_(numeric_type), data_(binary_ ? data.transpose() : data), n_components_(n_components)
		{
		}

		void initialize(const std::string &name, const std::string &numeric_type, const Eigen::MatrixXd &data, const int n_components = 1)
		{
			name_ = name;
			numeric_type_ = numeric_type;
			data_ = binary_ ? data.transpose() : data;
			n_components_ = n_components;
		}

		void write(std::ostream &os, uint64_t &offset) const
		{
			if (binary_)
			{
				os << "<DataArray type=\"" << numeric_type_ << "\" Name=\"" << name_ << "\" NumberOfComponents=\"" << n_components_ << "\" format=\"appended\" offset=\"" << offset << "\"/>\n";
				offset += appended_block_size();
			}
			else
			{
				os << "<DataArray type=\"" << numeric_type_ << "\" Name=\"" << name_ << "\" NumberOfComponents=\"" << n_components_ << "\" format=\"ascii\">\n";
				os << data_;
				os << "</DataArray>\n";
			}
		}

		void write_appended(std::ostream &os) const
		{
			const uint64_t size = payload_size();
			os.write(reinterpret_cast<const char *>(&size), sizeof(size));
			os.write(reinterpret_cast<const char *>(data_.data()), static_cast<std::streamsize>(size));
		}

		inline bool empty() const { return data_.size() <= 0; }
		inline uint64_t payload_size() const { return static_cast<uint64_t>(data_.size()) * sizeof(T); }
		inline uint64_t appended_block_size() const { return sizeof(uint64_t) + payload_size(); }

	private:
		std::string name_;
		bool binary_;
		std::string numeric_type_;
		Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> data_;
		int n_components_;
	};

	class VTUWriter : public ParaviewWriter
	{
	public:
		using ParaviewWriter::write_mesh;

		VTUWriter(bool binary = true);

		bool write_mesh(const std::string &path, const Eigen::MatrixXd &points, const Eigen::MatrixXi &cells, const CellType ctype) override;
		bool write_mesh(const std::string &path, const Eigen::MatrixXd &points, const std::vector<CellElement> &cells) override;

		void clear() override;

	protected:
		void add_scalar_field(const std::string &name, const Eigen::MatrixXd &data) override;
		void add_vector_field(const std::string &name, const Eigen::MatrixXd &data) override;

		void add_scalar_cell_field(const std::string &name, const Eigen::MatrixXd &data) override;
		void add_vector_cell_field(const std::string &name, const Eigen::MatrixXd &data) override;

	private:
		bool binary_;

		std::vector<VTKDataNode<double>> point_data_;
		std::string current_scalar_point_data_;
		std::string current_vector_point_data_;

		std::vector<VTKDataNode<double>> cell_data_;
		std::string current_scalar_cell_data_;
		std::string current_vector_cell_data_;

		void write_point_data(std::ostream &os, uint64_t &offset);
		void write_point_data_appended(std::ostream &os) const;
		void write_cell_data(std::ostream &os, uint64_t &offset);
		void write_cell_data_appended(std::ostream &os) const;
		void write_header(const int n_vertices, const int n_elements, std::ostream &os);
		void write_footer(std::ostream &os);
		void write_appended_data_header(std::ostream &os);
		void write_appended_data_footer(std::ostream &os);
		void write_file_footer(std::ostream &os);
		void write_points(const Eigen::MatrixXd &points, std::ostream &os, uint64_t &offset);
		void write_points_appended(const Eigen::MatrixXd &points, std::ostream &os) const;
		void write_cells(const Eigen::MatrixXi &cells, const CellType ctype, std::ostream &os, uint64_t &offset);
		void write_cells_appended(const Eigen::MatrixXi &cells, const CellType ctype, std::ostream &os) const;
		void write_cells(const std::vector<CellElement> &cells, std::ostream &os, uint64_t &offset);
		void write_cells_appended(const std::vector<CellElement> &cells, std::ostream &os) const;
	};
} // namespace paraviewo
