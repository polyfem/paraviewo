#pragma once

#include <Eigen/Dense>

namespace paraviewo
{
	class paraview_tags
	{
	private:
		// https://vtk.org/doc/nightly/html/vtkCellType_8h_source.html#l00069
		static const int VTK_VERTEX = 1;
		static const int VTK_LINE = 3;
		static const int VTK_TETRA = 10;
		static const int VTK_TRIANGLE = 5;
		static const int VTK_QUAD = 9;
		static const int VTK_HEXAHEDRON = 12;
		static const int VTK_POLYGON = 7;
		static const int VTK_POLYHEDRON = 42;

		static const int VTK_LAGRANGE_TRIANGLE = 69;
		static const int VTK_LAGRANGE_QUADRILATERAL = 70;

		static const int VTK_LAGRANGE_TETRAHEDRON = 71;
		static const int VTK_LAGRANGE_HEXAHEDRON = 72;

	public:
		inline static int VTKTagVolume(const int n_vertices, bool is_simplex, bool is_poly)
		{
			switch (n_vertices)
			{
			case 1:
				return VTK_VERTEX;
			case 2:
				return VTK_LINE;
			case 3:
				return VTK_TRIANGLE;
			case 4:
				return VTK_TETRA;
			case 8:
				return VTK_HEXAHEDRON;
			default:
				if (is_poly)
					return VTK_POLYHEDRON;
				if (is_simplex)
					return VTK_LAGRANGE_TETRAHEDRON;
				else
					return VTK_LAGRANGE_HEXAHEDRON;
			}
		}

		inline static int VTKTagPlanar(const int n_vertices, bool is_simplex, bool is_poly)
		{
			switch (n_vertices)
			{
			case 1:
				return VTK_VERTEX;
			case 2:
				return VTK_LINE;
			case 3:
				return VTK_TRIANGLE;
			case 4:
				return VTK_QUAD;
			default:
				if (is_poly)
					return VTK_POLYGON;

				if (is_simplex)
					return VTK_LAGRANGE_TRIANGLE;
				else
					return VTK_LAGRANGE_QUADRILATERAL;
			}
		}
	};

	class ParaviewWriter
	{
	public:
		ParaviewWriter(){};
		virtual ~ParaviewWriter(){};

		virtual bool write_mesh(const std::string &path, const Eigen::MatrixXd &points, const Eigen::MatrixXi &cells) = 0;
		virtual bool write_mesh(const std::string &path, const Eigen::MatrixXd &points, const std::vector<std::vector<int>> &cells, const bool is_simplicial, const bool has_poly) = 0;

		void add_field(const std::string &name, const Eigen::MatrixXd &data)
		{
			using std::abs;

			Eigen::MatrixXd tmp;
			tmp.resizeLike(data);

			for (long i = 0; i < data.size(); ++i)
				tmp(i) = abs(data(i)) < 1e-16 ? 0 : data(i);

			if (tmp.cols() == 1)
				add_scalar_field(name, tmp);
			else
				add_vector_field(name, tmp);
		}

		void add_cell_field(const std::string &name, const Eigen::MatrixXd &data)
		{
			using std::abs;

			Eigen::MatrixXd tmp;
			tmp.resizeLike(data);

			for (long i = 0; i < data.size(); ++i)
				tmp(i) = abs(data(i)) < 1e-16 ? 0 : data(i);

			if (tmp.cols() == 1)
				add_scalar_cell_field(name, tmp);
			else
				add_vector_cell_field(name, tmp);
		}

		virtual void clear() = 0;

	protected:
		virtual void add_scalar_field(const std::string &name, const Eigen::MatrixXd &data) = 0;
		virtual void add_vector_field(const std::string &name, const Eigen::MatrixXd &data) = 0;

		virtual void add_scalar_cell_field(const std::string &name, const Eigen::MatrixXd &data) = 0;
		virtual void add_vector_cell_field(const std::string &name, const Eigen::MatrixXd &data) = 0;
	};
} // namespace paraviewo
