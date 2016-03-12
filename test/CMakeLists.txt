set(TESTS
    internal/xml_parser.cpp
    config.cpp
    inheritance.cpp
    extra/string.cpp
    time.cpp
    logging/threshold_filter.cpp
    internal/dag.cpp
    endian.cpp
)

message(STATUS "Compile LMS tests")
add_executable(lmstest ${TESTS})
target_link_libraries(lmstest PRIVATE lmscore gtest gtest_main)
add_test(LMS lmstest)
