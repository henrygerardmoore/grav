# -*- coding: utf-8 -*-
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class GameConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    def requirements(self):
        self.requires("eigen/3.4.0")
        self.requires("entt/3.13.0")
        self.requires("opengl/system")
        self.requires("sfml/2.6.1")
        self.requires("quill/3.5.1")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
        self.options["sfml/*"].audio = False

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.build()
