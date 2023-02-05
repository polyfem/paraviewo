#include "VTMWriter.hpp"

namespace paraviewo
{
	VTMWriter::VTMWriter(const double t)
	{
		vtm_.InsertEndChild(vtm_.NewDeclaration());

		tinyxml2::XMLElement *root = vtm_.NewElement("VTKFile");
		vtm_.InsertEndChild(root);
		root->SetAttribute("type", "vtkMultiBlockDataSet");
		root->SetAttribute("version", "1.0");

		tinyxml2::XMLElement *multiblock = root->InsertNewChildElement("vtkMultiBlockDataSet");

		tinyxml2::XMLElement *data_array = root->InsertNewChildElement("FieldData")->InsertNewChildElement("DataArray");
		data_array->SetAttribute("type", "Float32");
		data_array->SetAttribute("Name", "TimeValue");
		data_array->InsertNewText(std::to_string(t).c_str());
	}

	void VTMWriter::add_dataset(const std::string &block_name, const std::string &name, const std::string &file_name)
	{
		tinyxml2::XMLElement *root = vtm_.FirstChildElement("VTKFile");
		tinyxml2::XMLElement *multiblock = root->FirstChildElement("vtkMultiBlockDataSet");
		tinyxml2::XMLElement *block = multiblock->InsertNewChildElement("Block");
		block->SetAttribute("name", block_name.c_str());
		tinyxml2::XMLElement *dataset = block->InsertNewChildElement("DataSet");
		dataset->SetAttribute("name", name.c_str());
		dataset->SetAttribute("file", file_name.c_str());
	}

	void VTMWriter::save(const std::string &file_name)
	{
		vtm_.SaveFile(file_name.c_str());
	}
} // namespace paraviewo