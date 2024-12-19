# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)
cmake_policy(SET CMP0011 NEW)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

# Use shared libraries from current directory on Linux (same behavior as on Windows)
SET(CMAKE_INSTALL_RPATH "$ORIGIN")

if(CMAKE_CXX_STANDARD LESS 17)
    message(WARNING "The FEP Participant library requires CMAKE_CXX_STANDARD >= 17")
endif()

file(STRINGS version
    VERSION
    LIMIT_COUNT 1
)
project(fep3-participant-library VERSION ${VERSION})
set(FEP3_PARTICIPANT_LIBRARY fep3_participant)

set(FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(FEP3_PARTICIPANT_LIBRARY_VERSION "${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_PATCH}")

# Enable strict compiler warnings
if(MSVC)
    # TODO /WD4100 should be removed when ODAUTIL-167 is fixed
    # 4251 is currently deactivated because some dll exported classes use std types within their interface (e.g. ComponentRegistry)
    add_compile_options(/W4 /WX /wd4251 /wd4100)
else()
    # TODO -Wno-comment should be removed when ODAUTIL-169 is fixed
    add_compile_options(-Wall -Wno-unknown-pragmas -Wno-reorder -Werror -Wextra -pedantic -Wno-comment)
endif()

include(scripts/cmake/enable_multicore_compilation.cmake)
include(scripts/cmake/use_integrated_debug_symbols.cmake)
include(scripts/cmake/set_symbol_visibility.cmake)

# Enable project folder structure for Visual Studio IDE
set_property(GLOBAL PROPERTY USE_FOLDERS true)

### Product specific
set(fep3_participant_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_FEP3_PARTICIPANT_LIBRARY true)
set(BETA_BUILD false CACHE BOOL "Mark as beta")

# some settings need to be set explicitly for QNX
if (UNIX)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
    set(CMAKE_SKIP_BUILD_RPATH  OFF)
endif(UNIX)

if (FEP3_USE_RTIDDS)
    set(fep3_participant_use_rtidds ON)
else()
    set(fep3_participant_use_rtidds OFF)
endif()

option(fep3_participant_cmake_enable_tests
       "Enable functional tests - requires googletest (default: OFF)" OFF)

option(fep_participant_arc_enable
       "Enable software architecture documentation build (default: OFF)" OFF)

# -Og is needed for ABI Dumper
if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
    add_compile_options("$<$<CONFIG:DEBUG>:-Og>")
    message(STATUS "Disable optimizations")

    option(fep3_participant_enable_code_coverage
           "Enable code coverage report with gcov (default: OFF)" OFF)
    if(fep3_participant_enable_code_coverage)
        message(STATUS "Enabling gcov flags")
        # See: https://gcc.gnu.org/onlinedocs/gcc-7.5.0/gcc/Instrumentation-Options.html
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
endif()
   
################################################################################
### Setting up packages
################################################################################

# compensate for the missing platform if building locally
if(NOT DEFINED PLATFORM)
    set(PLATFORM "developer")
endif(NOT DEFINED PLATFORM)

# add subdirectories core
if (fep_participant_arc_enable)
    add_subdirectory(doc/arc)
    
    install(
        FILES
            README.md
        DESTINATION
            .
    )
    install(
        FILES
            LICENSE.md
            doc/license/LicenseNotes.md
        DESTINATION
            doc/license
    )
    
else()

    #this is needed for teh pdb file while installed
    set(fep3_participant_pdb_version_str ${FEP3_PARTICIPANT_LIBRARY_VERSION_MAJOR}.${FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR})
    include(src/fep3/fep3_participant-macros.cmake)
    add_subdirectory(src)
    add_subdirectory(3rdparty/lssdp-cpp)

    install(
        FILES
            README.md
        DESTINATION
            .
    )
    install(
        FILES
            LICENSE.md
            doc/license/LicenseNotes.md
        DESTINATION
            doc/license
    )

    if(fep3_participant_cmake_enable_tests OR fep3_participant_export_test_targets)
        if(fep3_participant_cmake_enable_tests)
            # we have private tests that are built/run during the product build
            enable_testing()
            set(fep3_participant_cmake_integrated_tests ON)
        endif()
        add_subdirectory(test)
    endif()

    # install content from include directory
    install(
        DIRECTORY
            include
        DESTINATION
            ./
    )
endif()
