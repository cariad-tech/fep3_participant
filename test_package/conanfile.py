# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

from conans import ConanFile
from contextlib import contextmanager


class TestPackageFepParticipant(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    python_requires = "CoRTEXPythonRequiresHelper/2.4.0@fep/stable"

    _cprh = None
    _name = None

    @property
    def cprh(self):
        if not self._cprh:
            self._cprh = self.python_requires["CoRTEXPythonRequiresHelper"].module.Default
        return self._cprh

    @property
    def package_name(self):
        if not self._name:
            # If no further (build_)requirements are defined, the package to test is the only requirement
            # Extract the package name under test (e.g. 'fep_sdk_participant', 'fep_sdk_participant_arc', etc.)
            self._name = list(self.requires)[0]
        return self._name
    
    def build(self):
        pass
        
    def test(self):
        from conans import tools
        # self.name in test conanfiles is None, but cannot be set via set_name() method
        # Injecting here works and must be set, because install complete helper accesses it
        self.name = self.package_name
        # self.package_folder in test conanfiles is not the product under test package folder
        # inject the product under test package_folder and revert to original afterwards
        with self.change_package_folder(), tools.chdir(self.package_folder):
            # Use the reference file according to the product name under test
            custom_product_descriptor = {
                "source": { "install_complete_dir": f"install_complete/{self.name}" }
            }
            self.cprh.package__test_install_complete(self, custom_product_descriptor)

    @contextmanager
    def change_package_folder(self) -> None:
        package_folder = self.package_folder
        product_package_folder = self.deps_cpp_info[self.name].rootpath
        self.folders.set_base_package(product_package_folder)
        try:
            yield
        finally:
            self.folders.set_base_package(package_folder)