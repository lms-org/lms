from conans import ConanFile, CMake

class MathConan(ConanFile):
    name = "lms"
    version = "2.0"
    settings = "os", "compiler", "build_type", "arch"
    exports = "include/*","src/*","README.md","CMakeLists.txt"
    requires = "gtest/1.7.0@lasote/stable"
    generators = "cmake"

    def build(self):
        cmake = CMake(self.settings)
        self.run('cmake %s %s' % (self.conanfile_directory, cmake.command_line))
        self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        self.copy("*.h", dst="include", src="include")
        self.copy("*.so", dst="lib")

    def package_info(self):
        self.cpp_info.libs = ["lmscore"]
