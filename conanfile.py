from conans import ConanFile, CMake, tools
from conans.util import files
import yaml

class Qmg2ScConan(ConanFile):
    name = "qmg2sc"
    version = str(yaml.load(tools.load("settings.yml"))['project']['version'])
    license = "MIT"
    url = "https://git.hiventive.com/framework/qmg2sc.git"
    description = "QEMU-Machine-Generator to SystemC bridge"
    settings = "cppstd", "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False], "fPIE": [True, False]}
    default_options = "shared=False", "fPIC=False", "fPIE=False"
    generators = "cmake"
    exports = "settings.yml"
    exports_sources = "src/*", "CMakeLists.txt"
    requires = "gtest/1.8.0@bincrafters/stable", \
               "systemc/[~2.3.2]@hiventive/stable", \
               "module/0.2.0@hiventive/testing", \
               "communication/0.1.0@hiventive/testing", \
               "qmg/0.1.0@hiventive/testing"

    def _configure_cmake(self):
        cmake = CMake(self)
        if self.settings.os != "Windows":
            cmake.definitions["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC or self.options.fPIE
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
                
    def package_info(self):
        self.cpp_info.libs = ["qmg2sc"]
