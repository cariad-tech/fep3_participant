# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.


from pathlib import Path

from conans import python_requires  # @UnresolvedImport
import re
import os
import sys
import shutil
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

class FepParticipantAbiDump(FepSdkParticipantBase):
    name = "fep_sdk_participant_abi_dump"
    description = "Abi dump for the fep_participant package"
    plugin_dump_file = "fep_components_plugin_dump.dump"
    headers_file_name = "scripts/abi/abi_check_headers.txt"
    dump_folder = "abi_dump"

    def export(self):
        # headers are copied in scripts/abi and not scripts/abi/abi_dump since this is shared for
        # abi dumper and compliance checker
        self.copy("abi_check_headers.txt", dst = "scripts/abi", src="scripts/abi")
        self.copy("result_writer.py", dst = "scripts/abi", src="scripts/abi")
        self.copy("abi_dumper.py", dst = "scripts/abi", src="scripts/abi")
        self.copy("conanfile_fep_participant_base.py", dst="scripts/conan_helper", src="scripts/conan_helper")
        self.copy("conan_fep_participant_helper.py", dst="scripts/conan_helper", src="scripts/conan_helper")
        self.copy("version")

    def set_version(self):
        cortex_prh.tools.set_version(self, self.recipe_folder / Path("./version"))

    def build_requirements(self):
        self.build_requires(self.conan_data["build_requirements"]["abi_dumper"])
        self.build_requires("fep_sdk_participant/%s@%s/%s" % (self.version, self.user, self.channel))

    def build(self):
        dump_return_code, plugin_dump_file_path = create_dump(self)

        res_writer = ResultWriter(self.build_folder, abi_dump=True)

        if (dump_return_code == 0):
            self.output.info("Abi dumper finished successfully")
            self.output.info(f"Created abi dump file {plugin_dump_file_path}")
            res_writer.write_ok()
        else:
            self.output.warn(f"Abi dump failed with error code {dump_return_code}")
            res_writer.write_error("Abi dump failed, consult the dumper output.\n")

    def package_info(self):
        self.user_info.plugin_dump_file_name = self.dump_folder + "/" + self.plugin_dump_file

    def package(self):
        self.copy("*", dst = "abi_dump", src=os.path.join(self.build_folder, self.dump_folder))
        self.copy("*", dst = "test", src=os.path.join(self.build_folder, "test"))
