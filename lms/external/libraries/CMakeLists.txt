#Build libraries given by CmakeData
#Create CMakeData that looks like set ( LIBRARIES lib1 lib2 ...)
include("CMakeData.txt")

if (LIBRARIES)
        foreach (dec ${LIBRARIES})
                message ("Adding util_module ${dec}" )
                add_subdirectory ( ${dec} )
	endforeach()
else()
        message("No util_module defined")
endif()
