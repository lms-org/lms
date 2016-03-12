include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "-std=c++11")
check_cxx_source_compiles(
"
    #include <memory>
    int main() {
        constexpr auto CONSTANT = 2;
        static_assert(CONSTANT == 2, \"Expected 2\");

        auto i = std::make_shared<int>();

        return 0;
    }
" HAVE_CPP11)

if(HAVE_CPP11)
    MESSAGE(STATUS "Compiler supports C++11")
else()
    MESSAGE(FATAL_ERROR "Compiler does not support C++11")
endif()
