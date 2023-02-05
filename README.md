# ParaviewO

A simple interface to export paraview files. Currently it supports `.vtu` and `.hdf`.

The interface is in the `ParaviewWriter` class that provides

```
bool write_mesh(const std::string &path, const Eigen::MatrixXd &points, const Eigen::MatrixXi &cells)
bool write_mesh(const std::string &path, const Eigen::MatrixXd &points, const std::vector<std::vector<int>> &cells, const bool is_simplicial, const bool has_poly)
```
to write a mesh (either as matrices of vertices and cells or as a vector of vector if the mesh is not uniform).

The class supports adding fields
```
void add_field(const std::string &name, const Eigen::MatrixXd &data)
```
which *authomatically* writes a scalar or vector field. It also provides two specific methods (not recomended)
```
void add_scalar_field(const std::string &name, const Eigen::MatrixXd &data)
void add_vector_field(const std::string &name, const Eigen::MatrixXd &data)
```

## Example Usage

```
ParaviewWriter writer = VTUWriter(); // or HDF5VTUWriter()
writer.add_field("function", values);
writer.write_mesh("out.vtu", v, f);
```