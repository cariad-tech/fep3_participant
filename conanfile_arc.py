# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

from conan import ConanFile
from conan.tools.files import copy

from pathlib import Path

import os
import sys

required_conan_version = ">=1.46.0"

sys.path.append((Path(__file__).parent / "scripts/conan_helper").as_posix())
from conanfile_fep_participant_base import FepSdkParticipantBase

class FepParticipantArc(FepSdkParticipantBase):
    name = "fep_sdk_participant_arc"
    description = "FEP SDK Participant Library Software Architecture Documentation"

    python_requires = "CoRTEXPythonRequiresHelper/2.4.0@fep/stable",\
                      "CoRTEXCMakeGenerator/1.0.0@CoRTEX/stable"
 
    def init(self):
        self.prh_module = self.python_requires["CoRTEXPythonRequiresHelper"].module

    def configure(self):
        FepSdkParticipantBase.configure(self)
        self.custom_product_descriptor = {
            "source": {
                "doc_dirs": ["doc"]
            },
            "build": {
                "doc_dirs": ["doc"]
            }
        }

    def set_version(self):
        self.prh_module.tools.set_version(self, self.recipe_folder / Path("./version"))
      
    def build_requirements(self):
        self.tool_requires(self.conan_data["build_requirements"]["cmake"])
        self.tool_requires(self.conan_data["build_requirements"]["graphviz"])
        self.tool_requires(self.conan_data["build_requirements"]["doxygen"])
        self.tool_requires(self.conan_data["build_requirements"]["plantuml"])
       
    def build(self):
        additional_cmake_defs = None      
        additional_cmake_defs = {
            'fep_participant_arc_enable': True,
            'fep_participant_arc_draft_version': True, #ToDo: Set this to False when e.g. package channel is stable
            'fep_participant_plantuml_root_dir': self.deps_cpp_info["plantuml"].rootpath,
            'fep_participant_plantuml_input_root_dir': os.path.join(self.source_folder, "doc/arc/input/plantuml"),
            'fep_participant_plantuml_output_root_dir': os.path.join(self.build_folder, "doc/arc/input/images"),
            'fep_participant_doxygen_disable_warnings_as_errors': self.environ.DOXYGEN_DISABLE_WARNINGS_AS_ERRORS,
            'fep_participant_arc_python_executable': sys.executable,
            'fep_participant_arc_check_python_packages': "sphinx;sphinx_rtd_theme;breathe;sphinxmark;sphinxcontrib.spelling"
        }
                           
        self.prh_module.Default.build(self, self.custom_product_descriptor, additional_cmake_defs)

    def package(self):
        pass

    def export(self):
        copy(self, 'conanfile_fep_participant_base.py', os.path.join(self.recipe_folder, 'scripts', 'conan_helper'), os.path.join(self.export_folder, 'scripts', 'conan_helper'))
        copy(self, 'version', self.recipe_folder, self.export_folder)
        