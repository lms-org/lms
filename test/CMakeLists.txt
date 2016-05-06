set(TESTS
    internal/xml_parser.cpp
    config.cpp
    inheritance.cpp
    extra/string.cpp
    time.cpp
    logging/threshold_filter.cpp
    internal/dag.cpp
    internal/module_channel_graph.cpp
    endian.cpp
    datamanager.cpp
)

message(STATUS "Compile LMS tests")
add_executable(lmstest ${TESTS})
target_link_libraries(lmstest PRIVATE lmscore gtest gtest_main)
add_test(LMS lmstest)

if(COVERAGE)
    set_target_properties(lmstest PROPERTIES COMPILE_FLAGS "-fprofile-arcs -ftest-coverage -O0")
    set_target_properties(lmstest PROPERTIES LINK_FLAGS "-fprofile-arcs")
endif()
