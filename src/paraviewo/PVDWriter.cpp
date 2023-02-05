#include "PVDWriter.hpp"

#include <tinyxml2.h>
#include <fmt/core.h>

namespace paraviewo
{
	void PVDWriter::save_pvd(
		const std::string &name,
		const std::function<std::string(int)> &vtu_names,
		int time_steps, double t0, double dt, int skip_frame)
	{
		// https://www.paraview.org/Wiki/ParaView/Data_formats#PVD_File_Format

		tinyxml2::XMLDocument pvd;
		pvd.InsertEndChild(pvd.NewDeclaration());

		tinyxml2::XMLElement *root = pvd.NewElement("VTKFile");
		pvd.InsertEndChild(root);
		root->SetAttribute("type", "Collection");
		root->SetAttribute("version", "0.1");
		root->SetAttribute("byte_order", "LittleEndian");
		root->SetAttribute("compressor", "vtkZLibDataCompressor");

		tinyxml2::XMLElement *collection = root->InsertNewChildElement("Collection");

		for (int i = 0; i <= time_steps; i += skip_frame)
		{
			tinyxml2::XMLElement *dataset = collection->InsertNewChildElement("DataSet");
			dataset->SetAttribute("timestep", fmt::format("{:g}", t0 + i * dt).c_str());
			dataset->SetAttribute("group", "");
			dataset->SetAttribute("part", "0");
			dataset->SetAttribute("file", vtu_names(i).c_str());
		}

		pvd.SaveFile(name.c_str());
	}
} // namespace paraviewo