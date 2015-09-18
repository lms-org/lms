set(TESTS
    xml_parser.cpp
    module_config.cpp
    extra/string.cpp
    extra/time.cpp
    logging/threshold_filter.cpp
)

if(USE_GOOGLETEST)
    message(STATUS "Compile lmstest testing executable")
    add_executable(lmstest ${TESTS})
    target_link_libraries(lmstest PRIVATE lmscore gtest gtest_main)
    add_test(LMS lmstest)
endif()

