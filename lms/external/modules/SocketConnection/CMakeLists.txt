#Build runtimeModules given by CmakeData
include("CMakeData.txt")

if (SOCKET_MODULES)
	foreach (dec ${SOCKET_MODULES})
		message ("Adding runtime_module ${dec}" )
                add_subdirectory ( ${dec} )
	endforeach()
else()
	message("No runtime_module defined")
endif()
