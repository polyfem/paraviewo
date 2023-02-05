set(SOURCES
	ParaviewWriter.hpp
	VTMWriter.cpp
	VTMWriter.hpp
	HDF5VTUWriter.cpp
	HDF5VTUWriter.hpp
	VTUWriter.cpp
	VTUWriter.hpp
	base64Layer.hpp
	base64Layer.cpp
)

prepend_current_path(SOURCES)
target_sources(paraviewo PRIVATE ${SOURCES})
