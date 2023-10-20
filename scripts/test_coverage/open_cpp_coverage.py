# Copyright @ 2023 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

from conans import tools
from pathlib import Path, PureWindowsPath

class CppCodeCoverageTester():
    def __init__(self, conanfile):
        self.conanfile = conanfile
        self._coverage_report_folder: PureWindowsPath = None

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
                self._coverage_report_folder = PureWindowsPath(tools.get_env("OPENCPPCOVERAGE_OUTPUT"))
            else:
                self._coverage_report_folder = (PureWindowsPath(self.conanfile.package_folder) / "test/results/opencppcoverage")
        return self._coverage_report_folder

    def run(self):
        if self.conanfile.should_build:
            self.conanfile.output.info("OpenCppCoverage: Running code coverage")
            with tools.environment_append({"GTEST_OUTPUT": None}):
                self.__run_open_cpp_coverage()

    def __run_open_cpp_coverage(self):
        # FIXME: Use test_requirement() from CoRTEXPythonRequiresHelper.common module
        #        to check availability of cmake/ctest and opencppcoverage

        build_type = self.conanfile.settings.build_type
        build_folder = PureWindowsPath(self.conanfile.build_folder)
        source_folder = PureWindowsPath(self.conanfile.source_folder)
        tool_root_path = Path(self.conanfile.deps_cpp_info["opencppcoverage"].rootpath)
        opencppcoverage_exe = tool_root_path / "opencppcoverage/OpenCppCoverage.exe"

        self.conanfile.output.info(f"OpenCppCoverage: Build test folder: {build_folder}")
        # opencppcoverage requires the empty folder Plugins/Exporter
        # The conan recipe does its job but conan does not upload it
        # See: https://github.com/conan-io/conan/issues/8013
        with tools.chdir(opencppcoverage_exe.parent):
            Path("Plugins/Exporter").mkdir(parents=True, exist_ok=True)

        self.conanfile.output.info(f"OpenCppCoverage: Executing: {opencppcoverage_exe}")
        self.conanfile.output.info(f"command : {build_folder}")
        command = (f"{opencppcoverage_exe}"
                   f" --working_dir {build_folder}"
                   f" --export_type html:{self.coverage_report_folder}"
                   f" --export_type cobertura:{self.coverage_report_folder / 'opencppcoverage_result.xml'}"
                    " --cover_children --quiet"
                   f" --sources {source_folder / 'include'}"
                   f" --sources {source_folder / 'src'}"
                   f" --modules {build_folder}"
                       f" -- ctest {build_folder} --output-on-failure"
                       f" --repeat until-pass:5 -C {build_type}"
                  )

        self.conanfile.run(command)
        self.conanfile.output.info(
            f"OpenCppCoverage: Generated result files to {self.coverage_report_folder}")
