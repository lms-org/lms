#Build modules given by CmakeData
#write that file like: set ( MODULES mod1 mod2 ...)
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/CMakeData.txt")
include("CMakeData.txt")
endif()

if (MODULES)
	foreach (dec ${MODULES})
		message ("Adding runtime_module ${dec}" )
                add_subdirectory ( ${dec} )
	endforeach()
else()
	message("No runtime_module defined")
endif()
