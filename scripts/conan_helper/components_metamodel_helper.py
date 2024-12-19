# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

def validate_metamodel_file(conanfile, cortex_helper, file_path):
     # no python 3.8 for 32bit
     if (conanfile.settings.arch == "x86"):
        conanfile.output.info("Cannot validate Components metamodel description in x86 architecture, Python 3.10 required")
        return True
     activate_cmd = conanfile.deps_user_info["fep_metamodel"].ACTIVATE_SCRIPT_PATH
     activate_venv = activate_cmd if conanfile.settings.os == "Windows" else f". {activate_cmd}"
     try:
         conanfile.run(f"{activate_venv} && mm3_validator components {file_path}")
         conanfile.output.info(f"FEP Components description {file_path} is valid")
     except Exception:
         conanfile.output.warn(f"FEP Components description {file_path} is invalid")
         return False
     return True
