# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

from conans import python_requires #@UnresolvedImport
from conans import ConanFile, tools
from conans.errors import ConanException

from pathlib import Path
import os
import re
import sys
import subprocess
from subprocess import PIPE

sys.path.append((Path(__file__).parent / "scripts/conan_helper").as_posix())
from conanfile_fep_participant_base import FepSdkParticipantBase

#if we do not this conan create fails
file_dir = os.path.dirname(__file__)
sys.path.append(file_dir)

#append result_writer path
sys.path.append(file_dir + "/scripts/abi")
from result_writer import ResultWriter
from abi_dumper import create_dump

# get the Python requires packages
cortex_prh = python_requires("CoRTEXPythonRequiresHelper/2.4.0@fep/stable")
python_requires("CoRTEXCMakeGenerator/1.0.0@CoRTEX/stable")

class FepParticipantAbiCheck(FepSdkParticipantBase):
    name = "fep_sdk_participant_abi_check"
    description = "Abi compatibility test package for the fep_participant package"
    generators =  "txt"
    plugin_dump_file = "fep_components_plugin_dump.dump"
    reference_abi = "fep_sdk_participant_abi_dump"
    reference_abi_package = reference_abi + "/3.3.0@fep/FEPSDK_3693_abi_after_private_export"
    headers_file_name = "scripts/abi/abi_check_headers.txt"
    dump_folder = "abi_check"
    abi_script_path = "scripts/abi/abi_check"

    def export(self):
        # abi dumper and compliance checker
        self.copy("abi_dumper.py", dst = "scripts/abi", src="scripts/abi")
        self.copy(self.headers_file_name, dst = "scripts/abi", src="scripts/abi")
        self.copy("result_writer.py", dst = "scripts/abi", src="scripts/abi")
        self.copy("conanfile_fep_participant_base.py", dst="scripts/conan_helper", src="scripts/conan_helper")
        self.copy("version")

    def set_version(self):
        cortex_prh.tools.set_version(self, self.recipe_folder / Path("./version"))

    def build_requirements(self):
        self.build_requires(self.reference_abi_package)
        self.build_requires(self.conan_data["build_requirements"]["abi_dumper"])
        self.build_requires(self.conan_data["build_requirements"]["abi_compliance_checker"])
        self.build_requires("fep_sdk_participant/%s@%s/%s" % (self.version, self.user, self.channel))

    def package(self):
        self.copy("*", dst = "abi_check", src=os.path.join(self.build_folder, self.dump_folder))
        self.copy("*", dst = "test", src=os.path.join(self.build_folder, "test"))

    def build(self):
        dump_return_code, plugin_dump_file_path = create_dump(self)
        

        res_writer = ResultWriter(self.build_folder, abi_dump=True, abi_check=False)
        if (dump_return_code == 0):
            self.output.info("Abi dumper finished successfully")
            res_writer.write_ok()
        else:
            self.output.warn(f"Abi dumper finished with error code {dump_return_code}")
            res_writer.write_error("Abi dump failed, consult the dumper output.\n")
            return

        assert os.path.isfile(plugin_dump_file_path), "plugin abi dump file not found"

        part_plugin_lib = 'libfep_components_plugin.so'

        # this is where the reference ABIs are
        ref_dump_file_volume_path = os.path.join(
            self.deps_cpp_info[self.reference_abi].rootpath,
            self.deps_user_info[self.reference_abi].plugin_dump_file_name)
        assert os.path.isfile(ref_dump_file_volume_path), "reference abi dump file not found"

        compliance_report_path = os.path.join(self.build_folder, self.dump_folder, "report_plugin.html")
        abi_check_path = os.path.join(
            self.deps_cpp_info["abi_compliance_checker"].rootpath,
            self.deps_user_info["abi_compliance_checker"].command_path)
        assert os.path.isfile(abi_check_path), "abi checker executable not found"
        abi_check_command = f"{abi_check_path} -l {part_plugin_lib} -old {ref_dump_file_volume_path} -new {plugin_dump_file_path} -report-path {compliance_report_path}"

        check_return_code = self.run(
            f"{abi_check_command}",
            ignore_errors=True)

        res_writer = ResultWriter(self.build_folder, abi_dump=False, abi_check=True)
        if (check_return_code == 0):
            self.output.info("Abi checker finished successfully")
            res_writer.write_ok()
        else:
            self.output.warn(f"Abi checker finished with error code {check_return_code}")
            res_writer.write_error("Abi check failed, consult the checker output.\n")
