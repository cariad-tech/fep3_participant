# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

find_package(Doxygen REQUIRED)

# directory for doxygen and sphinx config files which were preprocessed by cmake
set(DOXYGEN_INPUTS "${CMAKE_SOURCE_DIR}/include")

option(fep_participant_doxygen_disable_warnings_as_errors "Whether or not to set doxygen WARN_AS_ERROR (Default: ON)" ON)

set(DOXYGEN_WARN_IF_UNDOCUMENTED "YES")

if(fep_participant_doxygen_disable_warnings_as_errors)
    set(DOXYGEN_WARNINGS_AS_ERRORS "NO")
    message(WARNING "DOXYGEN_WARNINGS_AS_ERRORS is deactivated")
else()
    set(DOXYGEN_WARNINGS_AS_ERRORS "YES")
endif()

set(DOXYGEN_EXCLUDES "\" \"" )
#set(DOXYGEN_EXCLUDE_PATTERNS "" )

#Set doxygen output to a subfolder of html output (required for github pages)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/output/html)
# Make directory required for making directories recursively
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output/html)

set(DOXYGEN_XML_OUTPUT_DIR ${DOXYGEN_OUTPUT_DIR}/doxygen_xml)
set(DOXYGEN_PREDEFINED "\"FEP_RPC_IID(iid, defaultname)= \" \"FEP_COMPONENT_IID(iid)= \" \"FEP3_PARTICIPANT_EXPORT= \"")

# Set project name for sphinx configuration
set(DOC_ARC_PROJECT_NAME "FEP SDK Participant Library\\nSoftware Architecture Documentation")
# Set draft version (=True/False) for sphinx configuration
set(DOC_ARC_DRAFT_VERSION ${fep_participant_arc_draft_version})

# Configure sphinx and doxygen
configure_file(input/doxygen.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/input/doxygen.cfg)
configure_file(input/conf.py.in ${CMAKE_CURRENT_BINARY_DIR}/input/conf.py)

# This is to avoid double documentation build on install
set(DOC_ARC_GENERATED_FLAG_FILE ${CMAKE_CURRENT_BINARY_DIR}/doc_generated)
add_custom_target(ARC_DOC_DEV_${PROJECT_NAME} ALL
    DEPENDS ${DOC_ARC_GENERATED_FLAG_FILE}
)

find_program(SPHINX_BUILD_EXE sphinx-build REQUIRED)

add_custom_command(OUTPUT ${DOC_ARC_GENERATED_FLAG_FILE}
    # checks if all necessary python packages are available for building the documentation
    COMMAND ${fep_participant_arc_python_executable} ${CMAKE_SOURCE_DIR}/scripts/doc_helper/check_build_requirements.py -p "${fep_participant_arc_check_python_packages}"
    # copy input to build directory
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/input ${CMAKE_CURRENT_BINARY_DIR}/input
    # render plantuml diagrams
    COMMAND ${CMAKE_COMMAND} -DPLANTUML_INPUT_DIR=${fep_participant_plantuml_input_root_dir} -DPLANTUML_OUTPUT_DIR=${fep_participant_plantuml_output_root_dir} -DPLANTUML_PACKAGE_DIR=${fep_participant_plantuml_root_dir} -P ${CMAKE_SOURCE_DIR}/scripts/cmake/render_plantuml_diagrams.cmake 
    # build doxygen
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/input/doxygen.cfg
    # check spelling
    COMMAND ${SPHINX_BUILD_EXE} -b spelling -c ${CMAKE_CURRENT_BINARY_DIR}/input ${CMAKE_CURRENT_BINARY_DIR}/input ${CMAKE_CURRENT_BINARY_DIR}/output
    # build sphinx html 
    COMMAND ${SPHINX_BUILD_EXE} -b html -c ${CMAKE_CURRENT_BINARY_DIR}/input ${CMAKE_CURRENT_BINARY_DIR}/input ${CMAKE_CURRENT_BINARY_DIR}/output/html
    # And mark that documentation as created
    COMMAND ${CMAKE_COMMAND} -E touch ${DOC_ARC_GENERATED_FLAG_FILE}
    WORKING_DIRECTORY .
    COMMENT "Build software architecture documentation"
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output/html/
    DESTINATION doc/html
    PATTERN "doxygen_xml" EXCLUDE
    PATTERN ".doctrees" EXCLUDE
    PATTERN ".buildinfo" EXCLUDE
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/input/fep3_participant_arc.html
    DESTINATION doc
)
