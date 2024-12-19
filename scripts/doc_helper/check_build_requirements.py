#!/usr/bin/env python3
# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

import sys
import importlib.util
import argparse

# Define a custom argument type for a list of strings
def list_of_packages(arg):
    return arg.split(';')

def main(packages):   
    """
    checks if all necessary python packages are available for building the documentation
    """
    print("check python packages for building the documentation: ", packages)
    unresolved_packages = []
    for package in packages:
        if (spec := importlib.util.find_spec(package)) is None:
            unresolved_packages.append(package)
    
    if len(unresolved_packages) > 0:
        print("Necessary python packages for building sphinx documentation not found. Please install python packages %s (e.g. pip install <package>)" % unresolved_packages)
        return 1
    else:
        return 0

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-p', '--package', help='semicolon seperated list of pyhton packages', type=list_of_packages, required=True)
    
    args = parser.parse_args(args=None if sys.argv[1:] else ['--help'])
    sys.exit(main(args.package))