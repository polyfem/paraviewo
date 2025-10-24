#pragma once

#include <Eigen/Dense>

namespace paraviewo
{
	enum class CellType
	{
		// === 0D / 1D ===
		Vertex,
		Line,

		// === 2D (planar) ===
		Triangle,
		Quadrilateral,
		Polygon,

		// === 3D (volume) ===
		Tetrahedron,
		Hexahedron,
		Wedge,
		Pyramid,
		Polyhedron,
	};

	class CellElement
	{
	public:
		std::vector<int> vertices;
		CellType ctype;
	};

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
	  	static const int VTK_WEDGE = 13;
		static const int VTK_PYRAMID = 14;
		static const int VTK_POLYGON = 7;
		static const int VTK_POLYHEDRON = 42;

  		static const int VTK_QUADRATIC_WEDGE = 26;
  		static const int VTK_BIQUADRATIC_QUADRATIC_WEDGE = 32;
		static const int VTK_QUADRATIC_PYRAMID = 27;
		
		static const int VTK_LAGRANGE_TRIANGLE = 69;
		static const int VTK_LAGRANGE_QUADRILATERAL = 70;

		static const int VTK_LAGRANGE_TETRAHEDRON = 71;
		static const int VTK_LAGRANGE_HEXAHEDRON = 72;

  		static const int VTK_LAGRANGE_WEDGE = 73;
  		static const int VTK_LAGRANGE_PYRAMID = 74;

	public:
		inline static int VTKTag(const int n_vertices, const CellType ctype)
		{
			switch (ctype)
			{
			// === 0D ===
			case CellType::Vertex:
				assert(n_vertices == 1);
				return VTK_VERTEX;

			// === 1D ===
			case CellType::Line:
				assert(n_vertices == 2);
				return VTK_LINE;

			// === 2D ===
			case CellType::Triangle:
				if (n_vertices == 3)
					return VTK_TRIANGLE;
				else
					return VTK_LAGRANGE_TRIANGLE;
			
			case CellType::Quadrilateral:
				if (n_vertices == 4)
					return VTK_QUAD;
				else
					return VTK_LAGRANGE_QUADRILATERAL;
			
			case CellType::Polygon:
				if (n_vertices > 4)
					return VTK_POLYGON;

			// === 3D ===
			case CellType::Tetrahedron:
				if (n_vertices == 4)
					return VTK_TETRA;
				else
					return VTK_LAGRANGE_TETRAHEDRON;
			
			case CellType::Hexahedron:
				if (n_vertices == 8)
					return VTK_HEXAHEDRON;
				else
					return VTK_LAGRANGE_HEXAHEDRON;
			
			case CellType::Wedge:
				if (n_vertices == 6)
					return VTK_WEDGE;
				else if (n_vertices == 15)
					return VTK_QUADRATIC_WEDGE;
				else if (n_vertices == 18)  // (n_vertices == 18 || n_vertices == 21);
					return VTK_BIQUADRATIC_QUADRATIC_WEDGE;
				else
					return VTK_LAGRANGE_WEDGE;
			
			case CellType::Pyramid:
				if (n_vertices == 5)
					return VTK_PYRAMID;
				else if (n_vertices == 13)
					return VTK_QUADRATIC_PYRAMID;
				else
					return VTK_LAGRANGE_PYRAMID;

			case CellType::Polyhedron:
				if (n_vertices >= 4)
					return VTK_POLYHEDRON;

			default:  // ?
				return VTK_TETRA;
			}
		}
	};

	class ParaviewWriter
	{
	public:
		ParaviewWriter(){};
		virtual ~ParaviewWriter(){};

		virtual bool write_mesh(const std::string &path, const Eigen::MatrixXd &points, const Eigen::MatrixXi &cells, const CellType ctype) = 0;
		virtual bool write_mesh(const std::string &path, const Eigen::MatrixXd &points, const std::vector<CellElement> &cells) = 0;

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
