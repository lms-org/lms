#Build runtimeModules given by CmakeData
include("CMakeData.txt")

if (UTIL_MODULES)
        foreach (dec ${UTIL_MODULES})
                message ("Adding util_module ${dec}" )
                add_subdirectory ( ${dec} )
	endforeach()
else()
        message("No util_module defined")
endif()
