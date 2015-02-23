#Add shared libs

file(GLOB_RECURSE SHARED_HEADER **/CMakeSharedHeaders.txt)
if (SHARED_HEADER)
        foreach(filepath ${SHARED_HEADER})
                get_filename_component(FOLDER ${filepath} DIRECTORY)

                message("Adding public headers ${FOLDER}")
                include_directories(${FOLDER})
	endforeach()
endif()
add_subdirectory(libraries)
add_subdirectory(modules)
