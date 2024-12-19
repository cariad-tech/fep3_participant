# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

from conans import python_requires #@UnresolvedImport
import os
import re
import sys
from pathlib import Path

sys.path.append((Path(__file__).parent / "scripts/conan_helper").as_posix())
from conanfile_fep_participant_base import FepSdkParticipantBase

# get the Python requires packages
qa_prh = python_requires("CoRTEXQaHelper/1.1.1@fep/stable")
cortex_prh = python_requires("CoRTEXPythonRequiresHelper/2.4.0@fep/stable")

class FepParticipantQa(FepSdkParticipantBase):
    name = "fep_sdk_participant_qa"
    description = "%s" % (name)
    generators = ["txt"]
    no_copy_source = False
    options = {}
    default_options = {}

    url = qa_prh.cortex_prh.Default.get_url_from_scm(FepSdkParticipantBase.scm)
    qa_profiles = {"Linux_armv8_gcc7":None, "Linux_x64_gcc7":None, "Windows_x64_vc142_VS2019":None}

    # use custom_product_descriptor as a property to read conan_refs dynamically from own properties
    _custom_product_descriptor= None

    def set_version(self):
        cortex_prh.tools.set_version(self, self.recipe_folder / Path("./version"))

    @property
    def custom_product_descriptor(self):
        if not self._custom_product_descriptor:
            self._custom_product_descriptor = {
                "qa": {
                    "requirements": {
                        "jira": {
                            "env_var" : "JIRA_REQS",
                            "jira_query" : 'project = "FEP SDK" and issuetype = Requirement and status != Rejected and component in ("Clock Discrete Sim Time", "Clock Service", "Clock Sync Service", "Component Registry", "Configuration Service", "Data Registry", "DDB DataReader", "DDB DataWriter","fep launcher", "fep participant library", "Participant", "RPC", "State Machine") and affectedVersion ~ "FEP SDK %s"' % self.version[:5],
                            "jira_user": os.getenv("JIRA_USR"),
                            "jira_pwd": os.getenv("JIRA_PSW"),
                            "jira_base_url": "https://devstack.vwgroup.com/jira"
                        }
                    },
                    "traceability": {
                        "tool_ref": "fep_trc/2.0.0@fep/stable",
                        "project": {
                            "conan_ref": None,  # conan reference, if the project file is not in this repository
                            "in_source": True,  # source reference of the conan_ref, if the file is in the source repository of the conan_ref
                            "project_rel_path": "test/traceability/FEP_Participant.rqtf",  # relative path to the project with file ending. Path prefix depends on "conan_ref" and "in_source"
                            "prjgen_template_dir": "",  # directory co    ntaining *.rqtftemplate if prjgen is used
                            "project_variant": None,  # apply this requirements filter
                        },
                        "references": [
                            {
                            "conan_ref": "fep_sdk_participant_tests/" + self.version + "@" + self.user + "/" + self.channel, #"FEPSDK_1940_update_the_fep_participant_library_to",
                            "profiles": self.qa_profiles,
                            "user_vars_to_env_vars": {"test_results_dirs_0": "FEP_SDK_PARTICIPANT_TEST_RESULTS"},  # maps conan user-vars from this references source/package dir to a chosen env-var name, to be used in the trc project.
                            }
                        ],
                        "reports": [
                            {
                                "name": "TestStatusReport",
                                "type": "TsrReport",
                                "options": {"REQTIFY_REPORT_TSR_CATEGORY": "Component Test"}
                            },
                           # To be fixed in FEPSDK-3700
                           #{
                           #    "name": "reportUnexecutedTestcases",
                           #    "type": "UnexecutedTestcaseReport",
                           #    "format": "xml",
                           #    "options": {"REQTIFY_REPORT_DOC_CATEGORY": "Component Test"}
                           #},
                            {
                            "name": "CoverageReport",
                            "type": "CovReport",
                            "options": {"REQTIFY_REPORT_COV_CATEGORY": "Software Requirements"}
                            }
                        ]
                    }
                }
            }
        return self._custom_product_descriptor


    def configure(self):
        FepSdkParticipantBase.configure(self)
        qa_prh.cortex_prh.Default.configure(self, self.custom_product_descriptor)

    def requirements(self):
        qa_prh.QAConanExtension.requirements(self, self.custom_product_descriptor)

    def build(self):
        qa_prh.QAConanExtension.build(self, self.custom_product_descriptor)

    def package(self):
        qa_prh.QAConanExtension.package(self, self.custom_product_descriptor)

    def package_info(self):
        qa_prh.QAConanExtension.package_info(self, self.custom_product_descriptor)

    def export(self):
        self.copy("conanfile_fep_participant_base.py", dst = "scripts/conan_helper", src="scripts/conan_helper")