////////////////////////////////////////////////////////////////////////////////
#include <paraviewo/VTUWriter.hpp>
#include <paraviewo/HDF5VTUWriter.hpp>

#include <Eigen/Dense>

#include <catch2/catch.hpp>
////////////////////////////////////////////////////////////////////////////////

using namespace paraviewo;

TEST_CASE("vtu_writer", "[utils]")
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

	Eigen::MatrixXi tris(1, 3);
	tris << 0, 1, 2;

	VTUWriter writer;
	writer.add_field("test", v);
	writer.write_mesh("test.vtu", pts, tris);
}

TEST_CASE("hdf5_writer", "[utils]")
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

	Eigen::MatrixXi tris(1, 3);
	tris << 0, 1, 2;

	HDF5VTUWriter writer;
	writer.add_field("test", v);
	writer.write_mesh("test.hdf", pts, tris);
}
