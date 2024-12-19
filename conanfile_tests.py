# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#

from conans import python_requires #@UnresolvedImport
from conans import tools
from pathlib import Path
from shutil import copyfile
from typing import Optional

import os
import sys

# get the Python requires packages
sys.path.append((Path(__file__).parent / "scripts/conan_helper").as_posix())
from conanfile_fep_participant_base import FepSdkParticipantBase

# get the Python requires packages
cortex_prh = python_requires("CoRTEXPythonRequiresHelper/2.4.0@fep/stable")
python_requires("CoRTEXCMakeGenerator/1.0.0@CoRTEX/stable")

class FepParticipantTests(FepSdkParticipantBase):
    name = "fep_sdk_participant_tests"
    description = "Functional test package for the fep_participant package"

    custom_product_descriptor = {
        "source": {
            "cmake_source_dir": "test",
            "doc_dirs": None
        },
        "build": {
            "doc_dirs": None
        }
    }

    def configure(self):
        FepSdkParticipantBase.configure(self)
        # force scm_branch of product dependencies to match mine
        self.options["fep_sdk_participant"].shared = self.options.shared

    def set_version(self):
        cortex_prh.tools.set_version(self, self.recipe_folder / Path("./version"))

    def build_requirements(self):
        self.build_requires(self.conan_data["build_requirements"]["cmake"])
        self.build_requires(self.conan_data["build_requirements"]["gtest"])
        # force channel and version of product dependencies to match mine
        self.build_requires("fep_sdk_participant/%s@%s/%s" % (self.version, self.user, self.channel))

    def requirements(self):
        self.requires(self.conan_data["requirements"]["boost"], private=True)

    def build(self):
        ## temporary copy conanbuildinfo content to install folder
        if self.should_configure:
            self._copy_buildinfo()

        additional_cmake_defs = {
                "fep3_participant_export_test_targets": "FALSE",
                "fep3_participant_cmake_enable_functional_tests": "TRUE",
                "CMAKE_FIND_PACKAGE_PREFER_CONFIG": "TRUE",
                "CMAKE_PROJECT_fep3-participant-tests_INCLUDE": self.install_folder + "/cortexbuildinfo.cmake",
                'GTest_ROOT': self.deps_cpp_info["gtest"].rootpath,
                'fep_sdk_participant_ROOT': self.deps_cpp_info["fep_sdk_participant"].rootpath
                }
        # For legacy compilers, C++14 has to be used, otherwise we might break ABI
        # with already shipped packages
        compiler_name = str(self.settings.compiler)
        compiler_ver = tools.Version(self.settings.compiler.version)
        if (compiler_name == "Visual Studio" and compiler_ver < 16) or\
           (compiler_name == "gcc" and compiler_ver < 7):
            additional_cmake_defs["CMAKE_CXX_STANDARD"] = 14

        self._build_and_test(additional_cmake_defs, cortex_prh.Default, "tester_result_functional_tests.xml")

    def package_info(self):
        cortex_prh.Default.package_info(self, self.custom_product_descriptor)

    def export(self):
        self.copy("conanfile_fep_participant_base.py", dst="scripts/conan_helper", src="scripts/conan_helper")
