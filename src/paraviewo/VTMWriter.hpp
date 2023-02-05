#pragma once

#include <tinyxml2.h>

#include <string>

namespace paraviewo
{
	class VTMWriter
	{
	public:
		VTMWriter(const double t = 0);
		void add_dataset(const std::string &block_name, const std::string &name, const std::string &file_name);
		void save(const std::string &file_name);

	private:
		tinyxml2::XMLDocument vtm_;
		tinyxml2::XMLElement *multiblock_;
	};
} // namespace paraviewo