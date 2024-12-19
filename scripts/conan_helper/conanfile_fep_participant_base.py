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
import os

class FepSdkParticipantBase(ConanFile):
    url = "https://devstack.vwgroup.com/bitbucket/scm/fepsdk/fep3_participant.git"
    license = "MPL-2.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CoRTEXCMakeGenerator", "cmake_paths", "cmake", "txt", "virtualenv"
    short_paths = True
    options = {"shared": [True, False], "use_rtidds": [True, False]}
    default_options = { "shared": True, "use_rtidds": True }
    default_channel = "testing"
    default_user = "fep"
    no_copy_source = True

    _environ: Optional["Environment"] = None

    scm = {
        "type": "git",
        "url": "auto",
        # Merges in PRs would create new commit hashes on every node, outdating the conanfile
        "revision": os.environ.get("CHANGE_ID") or "auto"
    }

    custom_product_descriptor = {
        "source": {
            "doc_dirs": None
        },
        "build": {
            "doc_dirs": None
        }
    }

    def configure(self):
        if self.in_local_cache:
            required_conan_version = ">= 1.28.0"

    @property
    def environ(self) -> "Environment":
        if not self._environ:
            self._environ = Environment()
        return self._environ

    ## temporary copy conanbuildinfo content to install folder
            ## temporary copy conanbuildinfo content to install folder
    def _copy_buildinfo(self):
        copyfile(self.source_folder + "/scripts/cmake/cortex_conanbuildinfo_integration.cmake.in",
                 self.install_folder + "/cortex_conanbuildinfo_integration.cmake")

    def _build_and_test(self, additional_cmake_defs, cortex_helper, test_result_filename):
        if self.should_configure:
            cortex_helper.build__cmake_configure(self, self.custom_product_descriptor, additional_cmake_defs)
        if self.should_build:
            cortex_helper.build__cmake_build(self)
        if self.should_install:
            cortex_helper.build__cmake_install(self)
        if self.should_test:
            self.run("ctest -C %s --output-junit %s --test-output-size-failed 0 --test-output-size-passed 0" %\
                     (self.settings.build_type,
                      os.path.join(self.package_folder, "test", "results", "function", test_result_filename)),
                     run_environment=True, ignore_errors=True)

class Environment:
    """Summarizes all environment variables for this recipe and gains easy
       access via properties.
    """

    def __init__(self) -> None:
        self._doxygen_disable_warnings_as_errors: Optional[bool] = None
        self._enable_documentation: Optional[bool] = None
        self._enable_functional_tests: Optional[bool] = None
        self._enable_file_formatting: Optional[bool] = None
        self._run_code_coverage_check: Optional[bool] = None
        self._run_black_duck_check: Optional[bool] = None
        self._ctest_test_dir: Optional[str] = None

    @property
    def DOXYGEN_DISABLE_WARNINGS_AS_ERRORS(self) -> bool:
        if self._doxygen_disable_warnings_as_errors is None:
            self._doxygen_disable_warnings_as_errors =\
                tools.get_env('DOXYGEN_DISABLE_WARNINGS_AS_ERRORS', False)
        return self._doxygen_disable_warnings_as_errors

    @property
    def ENABLE_DOCUMENTATION(self) -> bool:
        if self._enable_documentation is None:
            self._enable_documentation = tools.get_env('ENABLE_DOCUMENTATION', True)
        return self._enable_documentation

    @property
    def ENABLE_FUNCTIONAL_TESTS(self) -> bool:
        if self._enable_functional_tests is None:
            self._enable_functional_tests = tools.get_env('ENABLE_FUNCTIONAL_TESTS', False)
        return self._enable_functional_tests

    @property
    def ENABLE_FILE_FORMATTING(self) -> bool:
        if self._enable_file_formatting is None:
            self._enable_file_formatting = tools.get_env('ENABLE_FILE_FORMATTING', False)
        return self._enable_file_formatting

    @property
    def RUN_CODE_COVERAGE_CHECK(self) -> bool:
        if self._run_code_coverage_check is None:
            self._run_code_coverage_check = tools.get_env('RUN_CODE_COVERAGE_CHECK', False)
        return self._run_code_coverage_check

    @property
    def RUN_BLACK_DUCK_CHECK(self) -> bool:
        if self._run_black_duck_check is None:
            self._run_black_duck_check = tools.get_env('RUN_BLACK_DUCK_CHECK', False)
        return self._run_black_duck_check

    @property
    def CTEST_TEST_DIR(self) -> str:
        """
        This environment variable is used while running open_cpp_coverage.
        Points it to the build_folder you used to run conanfile.py
        """
        if self._ctest_test_dir is None:
            self._ctest_test_dir = tools.get_env('CTEST_TEST_DIR')
        return self._ctest_test_dir
