from conans import ConanFile, CMake

class LMSConan(ConanFile):
    name = "lms"
    version = "2.0"
    settings = "os", "compiler", "build_type", "arch"
    exports = "include/*","src/*","README.md","CMakeLists.txt","cmake/*","CMakeData.txt"
    requires = "gtest/1.8.0@lms/stable","pugixml/1.7@lms/stable","tclap/1.2.1@lms/stable","Protobuf/2.6.1@memsharded/testing"
    generators = "cmake","txt"

    def build(self):
        library_path = ":".join(self.deps_cpp_info.lib_paths)
        self.run('LD_LIBRARY_PATH=%s ./bin/protoc ../messages/messages.proto --proto_path=../messages --cpp_out="../include"' % library_path)
        self.run('./bin/protoc ../messages/messages.proto --proto_path=../messages --cpp_out="../include"')
        cmake = CMake(self.settings)
        self.run('cmake %s %s' % (self.conanfile_directory, cmake.command_line))
        self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        self.copy("*.h", dst="include", src="include")
        self.copy("*.so", dst="lib")
        self.copy("*/lms", dst=".")

    def package_info(self):
        self.cpp_info.libs = ["lmscore"]
        
    def imports(self):
        self.copy("protoc", "bin", "bin")
