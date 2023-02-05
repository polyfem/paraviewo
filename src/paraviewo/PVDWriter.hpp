#pragma once

#include <string>
#include <functional>

namespace praviewo
{
	class PVDWriter
	{
	public:
		static void save_pvd(
			const std::string &name,
			const std::function<std::string(int)> &vtu_names,
			int time_steps, double t0, double dt, int skip_frame);
	};
} // namespace praviewo
