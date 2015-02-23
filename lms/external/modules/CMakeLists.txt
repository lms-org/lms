#Build runtimeModules given by CmakeData
include("CMakeData.txt")

if (RUNTIME_MODULES)
	foreach (dec ${RUNTIME_MODULES})
		message ("Adding runtime_module ${dec}" )
                add_subdirectory ( ${dec} )
	endforeach()
else()
	message("No runtime_module defined")
endif()
