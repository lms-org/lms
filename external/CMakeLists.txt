add_library(pugixml STATIC         
    "pugixml-1.7/src/pugixml.cpp"
    "pugixml-1.7/src/pugixml.hpp"
    "pugixml-1.7/src/pugiconfig.hpp"
)
set_target_properties(pugixml PROPERTIES POSITION_INDEPENDENT_CODE 1)

add_subdirectory(googletest)
