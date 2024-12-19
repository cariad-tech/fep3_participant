# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

from conans import python_requires  # @UnresolvedImport
from conans import ConanFile, tools
from pathlib import Path
from shutil import copyfile
from typing import Optional
from os import environ

import sys

sys.path.append((Path(__file__).parent / "scripts/conan_helper").as_posix())
from open_cpp_coverage import CppCodeCoverageTester, OpenCppCoverage
from conanfile_fep_participant_base import FepSdkParticipantBase

# get the Python requires packages
cortex_prh = python_requires("CoRTEXPythonRequiresHelper/2.4.0@fep/stable")
python_requires("CoRTEXCMakeGenerator/1.0.0@CoRTEX/stable")

class FepParticipant(FepSdkParticipantBase):
    name = "fep_sdk_participant_test_coverage"
    description = "%s" % (name)
    # to disable auto deletion of imports after build step
    keep_imports = True

    _test_coverage_tool: Optional["OpenCppCoverage"] = None

    @property
    def test_coverage_tool(self) -> "OpenCppCoverage":
        if self.settings.os == "Linux":
            raise Exception("Run explicit test coverage on Windows platform or refer to SonarQube for Linux platform.")
        else:
            self._test_coverage_tool = OpenCppCoverage(self)
        return self._test_coverage_tool

    def configure(self):
        FepSdkParticipantBase.configure(self)
        self.options["boost"].shared = False

    def set_version(self):
        cortex_prh.tools.set_version(self, self.recipe_folder / Path("./version"))

    def build_requirements(self):
        self.test_coverage_tool.build_requirements()

    def requirements(self):
        self.test_coverage_tool.requirements()

    def build(self):
        self.test_coverage_tool.build()

    def package(self):
        self.test_coverage_tool.package()

    def export(self):
        self.copy("open_cpp_coverage.py", dst="scripts/conan_helper",
                  src="scripts/conan_helper")
        self.copy("conanfile_fep_participant_base.py", dst="scripts/conan_helper", src="scripts/conan_helper")
        self.copy("conan_fep_participant_helper.py", dst="scripts/conan_helper", src="scripts/conan_helper")
        self.copy("version")

    def package_info(self):
        cortex_prh.Default.package_info(self, self.custom_product_descriptor)
