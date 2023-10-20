#!/bin/bash

# Copyright @ 2023 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#
# This is a manual coverage test using gcov and lcov.
#
# ARG1 [optinal] dir to CMAKE_BINARY_DIR (default .)
#

BUILD_DIR="./build"

# ------------------------------------------------------------------------------

# function to plot usage info to the console
function usage {
    echo "Usage: "
    echo "  manual_coverage.sh optional [BUILD_DIR]"
    echo ""
    echo "Default BUILD_DIR=."
    echo "The BUILD_DIR must point on your CMAKE_BINARY_DIR."
    echo "The script will produce an HTML page for you after you have run \"make test\" on the project."
    exit 1
}

# ------------------------------------------------------------------------------

# arg handling
if [ "${#}" -ge 2 ]; then
    echo "Too many parameters were passed."
    usage
elif [ "${#}" -eq 1 ]; then
    if [ "${1}" = "-h" ] || [ "${1}" = "--help" ]; then
        usage
    else
        if [ ! -d "${1}" ]; then
            echo "Given build directory ${1} is not a valid directory!" >&2
            usage
        else
            BUILD_DIR="${1}"
        fi
    fi
fi
LCOV_DEBUG_DIR="${BUILD_DIR}/test/code_coverage"
OUTPUT_HTML_DIR="${LCOV_DEBUG_DIR}"

LCOV_INITIAL_FILE="${LCOV_DEBUG_DIR}/lcov_initial.tmp"
LCOV_COVERAGE_RAW_FILE="${LCOV_DEBUG_DIR}/lcov_coverage_raw.tmp"
LCOV_MERGED_FILE="${LCOV_DEBUG_DIR}/lcov_merged.tmp"
LCOV_OUTPUT_FILE="${LCOV_DEBUG_DIR}/fep_unittests"

#exit 1

echo "Using lcov..."
lcov --version
if [[ "${?}" -ne 0 ]]; then
    echo "Lcov could not be found on your computer. Please install it using \"sudo apt-get install lcov\" and try again." >&2
    exit 1
fi

echo "The configured build directory is: ${BUILD_DIR}"
echo "The output directory is: ${LCOV_DEBUG_DIR}"

echo "Creating working directory..."
mkdir -p ${LCOV_DEBUG_DIR} || exit 1
mkdir -p ${OUTPUT_HTML_DIR} || exit 1


echo "Initializing coverage project..."
lcov --capture --initial --directory ${BUILD_DIR} --output-file "${LCOV_INITIAL_FILE}" 1> /dev/null || exit 1

echo "Processing coverage data..."
echo "${LCOV_INITIAL_FILE}"

lcov --capture --directory ${BUILD_DIR} --output-file "${LCOV_COVERAGE_RAW_FILE}" 1> /dev/null || exit 1

lcov --add-tracefile "${LCOV_INITIAL_FILE}" \
     --add-tracefile "${LCOV_COVERAGE_RAW_FILE}" \
     --output-file "${LCOV_MERGED_FILE}" 1> /dev/null || exit 1

lcov --remove "${LCOV_MERGED_FILE}" \
              '/usr/*' \
              '/opt/*' \
              \
              '*/include/a_util/*' \
              '*/include/gtest/*' \
              '*/include/gmock/*' \
              '*/include/gmock/*' \
              '*/include/fep3/*' \
              \
              '*/test/*' \
              '*/build/*' \
              '*/clipp/*' \
              '*/dev_essential/*' \
              '*/rti_connext_dds/6.1.0/*' \
              '*/boost/*' \
              \
     --output-file "${LCOV_OUTPUT_FILE}" 1> /dev/null || exit 1

echo "Generating report in HTML format..."
genhtml -o "${OUTPUT_HTML_DIR}" "${LCOV_OUTPUT_FILE}" 1> /dev/null || exit 1

echo "Cleaning up..."
rm "${LCOV_INITIAL_FILE}" "${LCOV_COVERAGE_RAW_FILE}" "${LCOV_MERGED_FILE}" "${LCOV_OUTPUT_FILE}"
