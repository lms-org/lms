#include <sstream>
#include <string>
#include <vector>

#include "lms/extra/string.h"
#include "gtest/gtest.h"

TEST(string, trim) {
    using lms::extra::trim;

    // empty string
    ASSERT_EQ(std::string(""), trim(""));

    // whitespace string
    ASSERT_EQ(std::string(""), trim(" \t\n\r\x0b\x0c"));

    // with leading and trailing whitespace
    ASSERT_EQ(std::string("test"), trim(" test\r\n"));
}

TEST(string, split) {
    using lms::extra::split;
    typedef std::vector<std::string> V;

    // usual list
    ASSERT_EQ(V({"a", "b", "c"}), split("a,b,c", ','));

    // list with trailing split char
    ASSERT_EQ(V({"a", "b", "c"}), split("a:b:c:", ':'));

    // list with leading split char
    ASSERT_EQ(V({"", "a", "b", "c"}), split("|a|b|c", '|'));

    // empty list
    ASSERT_EQ(V(), split("", ';'));
}

TEST(string, dirname) {
    using lms::extra::dirname;

    // absolute path
    ASSERT_EQ(std::string("/tmp"), dirname("/tmp/test.file"));

    // absolute path with trailing slash
    ASSERT_EQ(std::string("/tmp"), dirname("/tmp/test.file/"));

    // relative path
    ASSERT_EQ(std::string("tmp"), dirname("tmp/test.file"));

    // relative path with trailing slash
    ASSERT_EQ(std::string("tmp"), dirname("tmp/test.file/"));

    // short absolute path
    ASSERT_EQ(std::string("/"), dirname("/tmp"));

    // short absolute path with trailing slash
    ASSERT_EQ(std::string("/"), dirname("/tmp/"));

    // short absolute path
    ASSERT_EQ(std::string("."), dirname("tmp"));

    // short absolute path with trailing slash
    ASSERT_EQ(std::string("."), dirname("tmp/"));

    // just the root
    ASSERT_EQ(std::string("/"), dirname("/"));

    // empty path
    ASSERT_EQ(std::string("."), dirname(""));
}

TEST(string, isAbsolute) {
    using lms::extra::isAbsolute;

    // relative path
    ASSERT_FALSE(isAbsolute("tmp/test.file"));

    // absolute path
    ASSERT_TRUE(isAbsolute("/tmp/test.file"));

    // empty path
    ASSERT_FALSE(isAbsolute(""));
}

TEST(string, safeGetline) {
    using lms::extra::safeGetline;

    std::istringstream iss(
    "linux line\n"
    "windows line\r\n"
    "\n" // empty line
    );

    std::string line;

    safeGetline(iss, line);
    ASSERT_EQ(std::string("linux line"), line);
    ASSERT_TRUE(!!iss);

    safeGetline(iss, line);
    ASSERT_EQ(std::string("windows line"), line);
    ASSERT_TRUE(!!iss);

    safeGetline(iss, line);
    ASSERT_EQ(std::string(""), line);
    ASSERT_TRUE(!!iss);

    safeGetline(iss, line);
    ASSERT_FALSE(!!iss);
}

TEST(string, lenOf) {
    using lms::extra::lenOf;

    ASSERT_EQ(size_t(4), lenOf("test"));
    ASSERT_EQ(size_t(0), lenOf(""));
}
