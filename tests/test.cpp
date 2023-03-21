////////////////////////////////////////////////////////////////////////////////
#include <paraviewo/VTUWriter.hpp>
#include <paraviewo/HDF5VTUWriter.hpp>

#include <Eigen/Dense>

#include <catch2/catch.hpp>
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
	writer.write_mesh(name, pts, tris);
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
