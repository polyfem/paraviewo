set(SOURCES
	ParaviewWriter.hpp
	VTMWriter.cpp
	VTMWriter.hpp
	VTUWriter.cpp
	VTUWriter.hpp
	PVDWriter.cpp
	PVDWriter.hpp
	base64Layer.hpp
	base64Layer.cpp
)

if(PARAVIEWO_WITH_HDF5)
	list(APPEND SOURCES HDF5VTUWriter.cpp HDF5VTUWriter.hpp)
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SOURCES})
target_sources(paraviewo PRIVATE ${SOURCES})
