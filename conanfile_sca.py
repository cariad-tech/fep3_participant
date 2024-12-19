# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

from conans import python_requires, ConanFile  # @UnresolvedImport
from pathlib import Path
from typing import Optional

from scripts.conan_helper.sonar_helper import sonar_scan
from scripts.conan_helper.conanfile_fep_participant_base import FepSdkParticipantBase

# get the Python requires packages
cortex_prh = python_requires("CoRTEXPythonRequiresHelper/2.4.0@fep/stable")
python_requires("CoRTEXCMakeGenerator/1.0.0@CoRTEX/stable")

class FepParticipant(FepSdkParticipantBase):
    name = "fep_sdk_participant_sca"
    description = "%s" % (name)
    # to disable auto deletion of imports after build step
    keep_imports = True

    @property
    def run_code_coverage_check(self) -> bool:
        if ((self.settings.os == "Linux") and
                (self.environ.RUN_CODE_COVERAGE_CHECK)):
            return True
        else:
            return False

    _clang_format_tester: Optional["ClangFormatTester"] = None

    @property
    def clang_format_tester(self) -> "ClangFormatTester":
        if not self._clang_format_tester:
            self._clang_format_tester = ClangFormatTester(self)
        return self._clang_format_tester

    def configure(self):
        FepSdkParticipantBase.configure(self)
        self.options["boost"].shared = False

    def set_version(self):
        cortex_prh.tools.set_version(
            self, self.recipe_folder / Path("./version"))

    def build_requirements(self):
        self.build_requires(self.conan_data["build_requirements"]["cmake"])
        self.build_requires(self.conan_data["build_requirements"]["gtest"])
        if self.options.use_rtidds:
            self.build_requires(
                self.conan_data["build_requirements"]["rti_connext_dds"])
        if self.environ.ENABLE_FILE_FORMATTING:
            self.build_requires(self.conan_data["build_requirements"]["llvm"])

    def requirements(self):
        self.requires(self.conan_data["requirements"]["boost"], private=True)
        self.requires(self.conan_data["requirements"]["dev_essential"])
        self.requires(self.conan_data["requirements"]["clipp"])

    def build(self):
        if self.should_configure:
            # temporary copy conanbuildinfo content to install folder
            self._copy_buildinfo()

        # clang-format tests
        status = "Executing" if self.environ.ENABLE_FILE_FORMATTING else "Skipping"
        self.output.info(f'{status} file formatting with clang format.')
        if self.environ.ENABLE_FILE_FORMATTING:
            self.clang_format_tester.run()

        additional_cmake_defs = {
            # configure boost according to required boost package
            # See: https://cmake.org/cmake/help/latest/module/FindBoost.html
            'Boost_USE_STATIC_LIBS': not self.options['boost'].shared,
            'Boost_USE_MULTITHREADED': self.options['boost'].multithreading,
            'CMAKE_PROJECT_fep3-participant-library_INCLUDE': \
            Path(self.install_folder) / 'cortexbuildinfo.cmake',
            'CMAKE_FIND_PACKAGE_PREFER_CONFIG': True,
            'FEP3_USE_RTIDDS': self.options.use_rtidds,
            'fep3_participant_cmake_enable_functional_tests': True,
            'fep3_participant_cmake_enable_documentation': False,
            'fep3_participant_cmake_enable_tests': True,
            'fep3_participant_export_test_targets': True,
            'fep3_participant_cmake_enable_private_tests': True,
            # Set CMake options to configure corresponding Boost:: imported targets
            'fep3_participant_cmake_boost_diagnostic_definitions': \
            self.options['boost'].diagnostic_definitions,
            'fep3_participant_cmake_boost_disable_autolinking': \
            not self.options['boost'].magic_autolink,
            'fep3_participant_cmake_boost_dynamic_linking': self.options['boost'].shared,
            'GTest_ROOT': self.deps_cpp_info["gtest"].rootpath,
        }

        if self.run_code_coverage_check:
            additional_cmake_defs['fep3_participant_enable_code_coverage'] = "TRUE"

        sonar_scan(self, cortex_prh, additional_cmake_defs,
                   self.run_code_coverage_check)

    def export(self):
        self.copy("conanfile_fep_participant_base.py",
                  dst="scripts/conan_helper", src="scripts/conan_helper")
        self.copy("sonar_helper.py", dst="scripts/conan_helper",
                  src="scripts/conan_helper")
        self.copy("version")

    def package_info(self):
        cortex_prh.Default.package_info(self, self.custom_product_descriptor)

class ClangFormatTester:
    def __init__(self, conanfile: ConanFile) -> None:
        self._conanfile = conanfile

    def run(self):
        conanfile = self._conanfile
        if conanfile.should_build:
            enable_file_formatting = "ON" if conanfile.environ.ENABLE_FILE_FORMATTING else "OFF"
            cmake_args = (f'-Dcortex_cmake_clang_format_working_directory={conanfile.source_folder} '
                          f'-Dcortex_cmake_enable_file_formatting="{enable_file_formatting}" '
                          '-Dcortex_cmake_clang_format_options="--Werror --dry-run --style=file --verbose --fallback-style=none"')
            cortex_clang_format_target_cmake = Path(
                conanfile.install_folder) / "cortex_clang_format_target.cmake"
            conanfile.run(
                f'cmake {cmake_args} -P {cortex_clang_format_target_cmake}')
