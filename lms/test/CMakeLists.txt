set(TESTS
    xml_parser.cpp
    extra/string.cpp
    extra/time.cpp
    type/module_config.cpp
    logging/threshold_filter.cpp
)

if(USE_GOOGLETEST)
    message("Compile lmstest testing executable")
    add_executable(lmstest ${TESTS})
    target_link_libraries(lmstest PRIVATE lmscore gtest gtest_main)
    add_test(LMS lmstest)
endif()

