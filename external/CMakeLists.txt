add_library(pugixml STATIC         
    "pugixml-1.7/src/pugixml.cpp"
    "pugixml-1.7/src/pugixml.hpp"
    "pugixml-1.7/src/pugiconfig.hpp"
)
set_target_properties(pugixml PROPERTIES POSITION_INDEPENDENT_CODE 1)

if(BUILD_TESTS)
#file(GLOB RESULT googletest)
#list(LENGTH ${RESULT} RES_LEN)
#if(RES_LEN GREATER 0)
    add_subdirectory(googletest)
#endif()
endif()
