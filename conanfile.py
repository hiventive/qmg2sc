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
    options = {"shared": [True, False],
               "fPIC": [True, False],
               "fPIE": [True, False],
               "target_aarch64": [True, False],
               "target_arm": [True, False]}
    default_options = {opt: False for opt in options}
    generators = "cmake"
    exports = "settings.yml"
    exports_sources = "src/*", "CMakeLists.txt"
    requires = "gtest/1.8.0@bincrafters/stable", \
               "systemc/[~2.3.2]@hiventive/stable", \
               "module/0.2.0@hiventive/testing", \
               "communication/0.1.0@hiventive/testing", \
               "qmg/0.6.0@hiventive/testing"

    def configure(self):
        if self.options.target_aarch64:
            self.options["qmg"].target_aarch64 = True
        if self.options.target_arm:
            self.options["qmg"].target_arm = True

    def _configure_cmake(self):
        cmake = CMake(self)
        target_list = []
        if self.options.target_aarch64:
            target_list.append("aarch64")
        if self.options.target_arm:
            target_list.append("arm")
        if self.settings.os != "Windows":
            cmake.definitions["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC or self.options.fPIE
        cmake.definitions["QMG2SC_TARGET_LIST"] = ";".join(target_list)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        libs_list = []
        if self.options.target_aarch64:
           libs_list.append("qmg2sc-aarch64")
        if self.options.target_arm:
            libs_list.append("qmg2sc-arm")
        self.cpp_info.libs = libs_list
