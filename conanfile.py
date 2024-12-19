# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

from conans import python_requires #@UnresolvedImport
from conans import ConanFile, tools
from contextlib import contextmanager
from pathlib import Path
from shutil import copyfile
from typing import Optional

import os
import sys

sys.path.append((Path(__file__).parent / "scripts/conan_helper").as_posix())
from conanfile_fep_participant_base import FepSdkParticipantBase
from open_cpp_coverage import CppCodeCoverageTester
from components_metamodel_helper import validate_metamodel_file

# get the Python requires packages
cortex_prh = python_requires("CoRTEXPythonRequiresHelper/2.4.0@fep/stable")
python_requires("CoRTEXCMakeGenerator/1.0.0@CoRTEX/stable")

class FepParticipant(FepSdkParticipantBase):
    name = "fep_sdk_participant"
    description = "%s" % (name)
    # to disable auto deletion of imports after build step
    keep_imports = True

    _cpp_code_coverage_tester: Optional["CppCodeCoverageTester"] = None

    exports = ("fep_sdk_native_components.mm3_components.yml")

    @property
    def cpp_code_coverage_tester(self) -> "CppCodeCoverageTester":
        #coverage in this conafile runs only in Jenkins and Windows (Linux is a seperate step)
        if ((not self._cpp_code_coverage_tester) and
            (self.settings.build_type == "Debug") and
            (self.settings.os == "Windows") and
            (self.environ.RUN_CODE_COVERAGE_CHECK)):
            self._cpp_code_coverage_tester = CppCodeCoverageTester(self)
        return self._cpp_code_coverage_tester

    @property
    def should_run_black_duck(self):
        node_can_run_black_duck = tools.get_env('JENKINS_NODE_TO_RUN_BLACK_DUCK_CHECK', None) != None
        black_duck_should_run = self.channel == 'stable' or \
        self.channel == 'integration' or \
        tools.get_env('RUN_BLACK_DUCK_CHECK', False)

        return black_duck_should_run and node_can_run_black_duck

    @contextmanager
    def pop_env_var(self, env_var: str) -> None:
        env_var_val = tools.get_env(env_var, "")
        if env_var_val:
            from os import environ
            environ.pop(env_var)
        try:
            yield
        finally:
            if env_var_val:
                environ[env_var] = env_var_val
    @property
    def version_without_build_number(self):
        return cortex_prh.tools.remove_build_number(self.version)

    def configure(self):
        FepSdkParticipantBase.configure(self)
        self.options["boost"].shared = False

    def set_version(self):
        cortex_prh.tools.set_version(self, self.recipe_folder / Path("./version"))

    def build_requirements(self):
        self.build_requires(self.conan_data["build_requirements"]["cmake"])
        self.build_requires(self.conan_data["build_requirements"]["gtest"])

        if self.options.use_rtidds:
            self.build_requires(self.conan_data["build_requirements"]["rti_connext_dds"])
        if self.cpp_code_coverage_tester:
            self.build_requires(self.conan_data["build_requirements"]["opencppcoverage"])
        if (self.settings.arch != "x86"):
            self.build_requires(self.conan_data["build_requirements"]["fep_metamodel"])

    def requirements(self):
        self.requires(self.conan_data["requirements"]["boost"], private=True)
        self.requires(self.conan_data["requirements"]["dev_essential"])
        self.requires(self.conan_data["requirements"]["clipp"])

    def build(self):
        additional_cmake_defs = None
        if self.should_configure:
            ## temporary copy conanbuildinfo content to install folder
            self._copy_buildinfo()

            additional_cmake_defs = {
                # configure boost according to required boost package
                # See: https://cmake.org/cmake/help/latest/module/FindBoost.html
                'Boost_USE_STATIC_LIBS': not self.options['boost'].shared,
                'Boost_USE_MULTITHREADED': self.options['boost'].multithreading,
                'CMAKE_PROJECT_fep3-participant-library_INCLUDE':\
                    Path(self.install_folder) / 'cortexbuildinfo.cmake',
                'CMAKE_FIND_PACKAGE_PREFER_CONFIG': True,
                'FEP3_USE_RTIDDS': self.options.use_rtidds,
                'fep3_participant_cmake_enable_functional_tests':\
                    self.environ.ENABLE_FUNCTIONAL_TESTS,
                'fep3_participant_cmake_enable_tests': True,
                'fep3_participant_export_test_targets': True,
                'fep3_participant_cmake_enable_private_tests': True,
                # Set CMake options to configure corresponding Boost:: imported targets
                'fep3_participant_cmake_boost_diagnostic_definitions':\
                    self.options['boost'].diagnostic_definitions,
                'fep3_participant_cmake_boost_disable_autolinking':\
                    not self.options['boost'].magic_autolink,
                'fep3_participant_cmake_boost_dynamic_linking': self.options['boost'].shared,
                'GTest_ROOT': self.deps_cpp_info["gtest"].rootpath,
            }

            # For legacy compilers, C++14 has to be used, otherwise we might break ABI
            # with already shipped packages
            compiler_name = str(self.settings.compiler)
            compiler_ver = tools.Version(self.settings.compiler.version)
            if (compiler_name == "Visual Studio" and compiler_ver < 16) or\
            (compiler_name == "gcc" and compiler_ver < 7):
                additional_cmake_defs['CMAKE_CXX_STANDARD'] = 14

            status = "Executing" if self.cpp_code_coverage_tester else "Skipping"
            self.output.info(f'{status} code coverage with OpenCppCoverage')

            status = "Executing" if self.should_run_black_duck else "Skipping"
            self.output.info(f'{status} Black Duck scan')

        if self.should_build:
            if not validate_metamodel_file(self, cortex_prh, Path(self.source_folder) / "fep_sdk_native_components.mm3_components.yml"):
                self.output.error("FEP Component Description is invalid. Skipping build process.")


        # Building package
        # If ninja is required, CONAN_CMAKE_GENERATOR=Ninja is injected
        # Force build with cmake using the default generator instead
        with self.pop_env_var("CONAN_CMAKE_GENERATOR"):
            self._build_and_test(additional_cmake_defs, cortex_prh.Default, "tester_result_private_tests.xml")

        self.__write_change_log()

        # Code coverage tester (Windows)
        if self.cpp_code_coverage_tester:
            self.cpp_code_coverage_tester.run()

        # Black Duck check
        if self.should_run_black_duck:
            self._scan_with_black_duck()

    def export(self):
        self.copy("open_cpp_coverage.py", dst="scripts/conan_helper", src="scripts/conan_helper")
        self.copy("conanfile_fep_participant_base.py", dst="scripts/conan_helper", src="scripts/conan_helper")
        self.copy("conan_fep_participant_helper.py", dst="scripts/conan_helper", src="scripts/conan_helper")
        self.copy("components_metamodel_helper.py", dst="scripts/conan_helper", src="scripts/conan_helper")
        self.copy("version")

    def package(self):
        self.copy("*", src=self.source_folder + "/doc/license/*", dst="doc/license")

        # Check if changelog file was generated during build
        if os.path.exists(os.path.join(self.build_folder, "doc/changelog.md")):
            # Package generated changelog file
            self.copy("changelog.md", src=os.path.join(self.build_folder, "doc"), dst="doc")
        else:
            # Package template changelog file
            self.copy("changelog.md", src=os.path.join(self.source_folder, "doc"), dst="doc")

    def package_info(self):
        cortex_prh.Default.package_info(self, self.custom_product_descriptor)

    def __write_change_log(self):
        generate_changelog = tools.get_env('JENKINS_URL', None) != None
        if generate_changelog:
            self.output.info("writing change log")
            cortex_prh_changelog_module = self.python_requires["CoRTEXPythonRequiresHelper"].module.ChangelogGenerator
            export_path = Path(self.build_folder) / "doc/changelog.md"
            export_path.parent.mkdir(parents=True, exist_ok=True)
            input_file =  Path(self.source_folder) / "doc/changelog.md"
            properties = cortex_prh_changelog_module.ChangelogGeneratorProperties(
                    atlassian_url = "https://devstack.vwgroup.com",
                    output_path = export_path,
                    input_file = input_file,
                    jira_project = "FEPSDK",
                    additional_jira_query = 'component not in ("fep system library", DOC, Examples)',
                    bitbucket_repo = "fep3_participant",
                    bitbucket_project = "FEPSDK",
                    jira_versions = [f"FEP SDK {self.version_without_build_number}"],
                    product_name= "FEP SDK Participant",
                    username = tools.get_env("JIRA_USR", None),
                    jira_api_token = tools.get_env("JIRA_PSW", None),
                    bitbucket_api_token = tools.get_env("BITBUCKET_PSW", None))
            cortex_prh_changelog_module.create(self, properties)
        else:
            self.output.info("Environment variable 'JENKINS_URL' not set. Building without doc/changelog.md")

    def _scan_with_black_duck(self):
        #  Run with snippet matching the full workspace:
        self.custom_product_descriptor["build"] = {
            "blackduck_project_name": "FEP_SDK",
            "blackduck_project_group": "fep",
            "blackduck_tags": ["vec", "tcc"],
            "blackduck_channel": ["testing","stable","integration", f"{self.channel}"],
            "blackduck_additional_args": [
                "--detect.blackduck.signature.scanner.copyright.search=true",
                "--detect.blackduck.signature.scanner.license.search=true",
                "--detect.blackduck.signature.scanner.snippet.matching=SNIPPET_MATCHING",
                "--detect.detector.search.depth=999",
                f"--detect.project.version.name='{self.name} {self.version} {self.channel}'",
                f"--detect.source.path={tools.get_env('WORKSPACE', '')}",
                "--detect.tools=\"DETECTOR\",\"SIGNATURE_SCAN\""
            ]
        }
        try:
           cortex_prh.BlackduckHelper.blackduck_scan(
            self, self.custom_product_descriptor, True)
        except Exception as error:
            self.output.warn(f"An exception occurred while running blackduck scan: {error}")
