# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

from conans import tools, ConanFile
import os
from pathlib import Path
import stat

# SonarQube build wrapper to gather information (required for SonarQube analysis) during product build
sonar_build_wrapper_data = {
    "Windows": {
        "file": {
            "url": "https://jfrog.devstack.vwgroup.com/artifactory/fepdev-thirdparty-generic-internal/sonar-build-wrapper/6.41/build-wrapper-win-x86.zip"
        },
        "path": "build-wrapper-win-x86",
        "exe": "build-wrapper-win-x86-64.exe"
    },
    "Linux": {
        "file": {
            "url": "https://jfrog.devstack.vwgroup.com/artifactory/fepdev-thirdparty-generic-internal/sonar-build-wrapper/6.41/build-wrapper-linux-x86.zip"
        },
        "path": "build-wrapper-linux-x86",
        "exe": "build-wrapper-linux-x86-64"
    }
}

# SonarQube scanner to analyze build data and upload results
sonar_scanner_data = {
    "Windows": {
        "file": {
            "url": "https://jfrog.devstack.vwgroup.com/artifactory/fepdev-thirdparty-generic-internal/sonar-scanner/4.2/sonar-scanner-cli-4.2.0.1873-windows.zip"
        },
        "path": "sonar-scanner-4.2.0.1873-windows"
    },
    "Linux": {
        "file": {
            "url": "https://jfrog.devstack.vwgroup.com/artifactory/fepdev-thirdparty-generic-internal/sonar-scanner/4.2/sonar-scanner-cli-4.2.0.1873-linux.zip"
        },
        "path": "sonar-scanner-4.2.0.1873-linux"
    }
}


def sonar_scan(conanfile: ConanFile, cortex_prh, additional_cmake_defs, run_code_coverage_check) -> None:
    recipe_os = str(conanfile.settings.os)
    tools.get(**sonar_scanner_data[recipe_os]["file"],
              auth=(tools.get_env("ARTIFACTORY_USR"), tools.get_env("ARTIFACTORY_PSW")))
    tools.get(**sonar_build_wrapper_data[recipe_os]["file"],
              auth=(tools.get_env("ARTIFACTORY_USR"), tools.get_env("ARTIFACTORY_PSW")))

    build_wrapper_out_dir = Path(conanfile.build_folder) / "sonar_build_output"
    build_wrapper_home = Path(conanfile.build_folder) / \
        sonar_build_wrapper_data[recipe_os]["path"]
    build_wrapper = build_wrapper_home / \
        sonar_build_wrapper_data[recipe_os]["exe"]

    sonar_scanner_home = Path(conanfile.build_folder) / \
        sonar_scanner_data[recipe_os]["path"]
    java_home = sonar_scanner_home / "jre"
    sonar_scanner = sonar_scanner_home / "bin" / "sonar-scanner"

    if recipe_os == "Linux":
        _make_executable(java_home / "bin" / "java")
        _make_executable(sonar_scanner)
        _make_executable(build_wrapper)

    cortex_prh.Default.build__cmake_configure(
        conanfile, conanfile.custom_product_descriptor, additional_cmake_defs)

    # Product build using SonarQube build wrapper
    conanfile.run(f"{build_wrapper.as_posix()} "
                  f"--out-dir {build_wrapper_out_dir} "
                  f"cmake --build {conanfile.build_folder} --config {conanfile.settings.build_type}")

    code_coverage_out_dir = os.path.join(
        conanfile.build_folder, "code_coverage")
    os.makedirs(code_coverage_out_dir, exist_ok=True)

    status = "Executing" if run_code_coverage_check else "Skipping"
    conanfile.output.info(f'{status} code coverage check.')
    if run_code_coverage_check:
        # Run tests to produce test coverage data
        conanfile.run("ctest -C %s --output-junit %s --test-output-size-failed 0 --test-output-size-passed 0" %
                      (conanfile.settings.build_type,
                       os.path.join(conanfile.build_folder, "test", "results", "function", "tester_result_functional_tests.xml")),
                      run_environment=True, ignore_errors=True)

        # Run gcov to translate `.gcda` files into `.gcov` readable by humans and SonarCloud.
        # --preserve-paths helps us avoid name clash for `.gcov` files corresponding to source files
        # with the same name but in different directories.
        conanfile.run(
            f"find .. -name '*.o' | xargs gcov --preserve-paths", cwd=code_coverage_out_dir)

    # Run SonarQube analysis
    conanfile.run(f"{sonar_scanner.as_posix()} -Dsonar.login={tools.get_env('SONAR_PSW')} "
                  f"-Dsonar.projectBaseDir={conanfile.source_folder} "
                  f"-Dsonar.projectVersion={_base_version(conanfile.version)} "
                  f"-Dsonar.cfamily.build-wrapper-output={build_wrapper_out_dir} "
                  f"-Dsonar.cfamily.gcov.reportsPath={code_coverage_out_dir}")


def _make_executable(exe: Path) -> None:
    exe.chmod(exe.stat().st_mode | stat.S_IEXEC)


def _base_version(version: str) -> str:
    if "+" in version:
        return version.split("+")[0]
    return version
