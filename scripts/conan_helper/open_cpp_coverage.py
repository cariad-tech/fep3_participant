# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

from conans import python_requires  # @UnresolvedImport
from conans import ConanFile, tools
from pathlib import Path, PureWindowsPath
from typing import Optional
from shutil import copyfile


class CppCodeCoverageTester():
    python_requires = "CoRTEXPythonRequiresHelper/2.4.0@fep/stable", \
                      "CoRTEXCMakeGenerator/1.0.0@CoRTEX/stable"

    def __init__(self, conanfile):
        self.conanfile = conanfile
        self._coverage_report_folder: PureWindowsPath = None

    @property
    def cprh(self):
        if not self._cprh:
            self._cprh = self.python_requires["CoRTEXPythonRequiresHelper"].module.Default
        return self._cprh

    @property
    def coverage_report_folder(self) -> PureWindowsPath:
        """Get the coverage report folder.

        If OPENCPPCOVERAGE_OUTPUT environment variable is defined, use this folder.
        Otherwise, use <package_folder>/test/result/opencppcoverage.

        Returns:
            PureWindowsPath: Path to the folder the coverage report is written to.
        """
        if not self._coverage_report_folder:
            if tools.get_env("OPENCPPCOVERAGE_OUTPUT"):
                self._coverage_report_folder = PureWindowsPath(
                    tools.get_env("OPENCPPCOVERAGE_OUTPUT"))
            else:
                self._coverage_report_folder = (PureWindowsPath(
                    self.conanfile.package_folder) / "test/results/opencppcoverage")
        return self._coverage_report_folder

    def run(self):
        if self.conanfile.should_build:
            self.conanfile.output.info(
                "OpenCppCoverage: Running code coverage")
            with tools.environment_append({"GTEST_OUTPUT": None}):
                self.__run_open_cpp_coverage()

    def __run_open_cpp_coverage(self):
        # FIXME: Use test_requirement() from CoRTEXPythonRequiresHelper.common module
        #        to check availability of cmake/ctest and opencppcoverage

        build_type = self.conanfile.settings.build_type
        build_folder = PureWindowsPath(self.conanfile.build_folder)
        source_folder = PureWindowsPath(self.conanfile.source_folder)
        tool_root_path = Path(
            self.conanfile.deps_cpp_info["opencppcoverage"].rootpath)
        opencppcoverage_exe = tool_root_path / "opencppcoverage/OpenCppCoverage.exe"

        if "test" in Path(build_folder).iterdir():
            ctest_folder = build_folder
        elif self.conanfile.environ.CTEST_TEST_DIR:
            ctest_folder = Path(self.conanfile.environ.CTEST_TEST_DIR)
            if not ctest_folder.is_dir():
                self.conanfile.output.error(f"OpenCppCoverage: {ctest_folder} is not a directory. "
                                            "Use a valid directory path for CTEST_TEST_DIR")
        else:
            self.conanfile.output.error(f"OpenCppCoverage: {build_folder} does not contain a subfolder test. "
                                        "Probably you're running with another build_folder where no tests were built. "
                                        "Try to build tests first and then point CTEST_TEST_DIR "
                                        "to the build_folder used to run conanfile.py.")
            ctest_folder = build_folder

        self.conanfile.output.info(f"OpenCppCoverage: Build test folder: {build_folder}")
        self.conanfile.output.info(f"OpenCppCoverage: ctest test folder: {ctest_folder}")
        # opencppcoverage requires the empty folder Plugins/Exporter
        # The conan recipe does its job but conan does not upload it
        # See: https://github.com/conan-io/conan/issues/8013
        with tools.chdir(opencppcoverage_exe.parent):
            Path("Plugins/Exporter").mkdir(parents=True, exist_ok=True)

        self.conanfile.output.info(f"OpenCppCoverage: Executing: {opencppcoverage_exe}")
        self.conanfile.output.info(f"OpenCppCoverage: Build test folder: {build_folder}")
        command = (f"{opencppcoverage_exe}"
                   f" --working_dir {build_folder}"
                   f" --export_type html:{self.coverage_report_folder}"
                   f" --export_type cobertura:{self.coverage_report_folder / 'opencppcoverage_result.xml'}"
                    " --cover_children --quiet"
                   f" --sources {source_folder / 'include'}"
                   f" --sources {source_folder / 'src'}"
                   f" --modules {ctest_folder}"
                   f" -- ctest --test-dir {ctest_folder}"
                    " --output-on-failure"
                   f" --repeat until-pass:5 -C {build_type}"
                   )

        self.conanfile.run(command)
        self.conanfile.output.info(
            f"OpenCppCoverage: Generated result files to {self.coverage_report_folder}")


class OpenCppCoverage:
    def __init__(self, conanfile: ConanFile) -> None:
        self._conanfile = conanfile

    _cpp_code_coverage_tester: Optional["CppCodeCoverageTester"] = None

    @property
    def cpp_code_coverage_tester(self) -> "CppCodeCoverageTester":
        if not self._cpp_code_coverage_tester:
            self._cpp_code_coverage_tester = CppCodeCoverageTester(
                self._conanfile)
        return self._cpp_code_coverage_tester

    def _is_jenkins_build(self) -> bool:
        return True if tools.get_env("JENKINS_URL") else False

    def build_requirements(self):
        if not self._is_jenkins_build():
            self._conanfile.build_requires("cmake/3.23.2@fep/stable")
            self._conanfile.build_requires(
                "opencppcoverage/0.9.9.0@fep/stable")

    def requirements(self):
        pass

    def build(self):
        # the opencpp coverage tool is run in the build step in jenkins
        if self._is_jenkins_build():
            self._conanfile.output.info(
                "Skipping code coverage, was run in build stage")
        else:
            self.cpp_code_coverage_tester.run()

    def package(self):
        self._conanfile.copy("*",
                             src=self.cpp_code_coverage_tester.coverage_report_folder,
                             dst="test_coverage_report")
