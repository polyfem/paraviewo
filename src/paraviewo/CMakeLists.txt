set(SOURCES
	ParaviewWriter.hpp
	VTMWriter.cpp
	VTMWriter.hpp
	HDF5VTUWriter.cpp
	HDF5VTUWriter.hpp
	VTUWriter.cpp
	VTUWriter.hpp
	PVDWriter.cpp
	PVDWriter.hpp
	base64Layer.hpp
	base64Layer.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SOURCES})
target_sources(paraviewo PRIVATE ${SOURCES})
