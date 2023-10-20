#
# Copyright @ 2021 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

if(NOT PROJECT_NAME)
    cmake_minimum_required(VERSION 3.17)
    project(fep3-participant-tests)

    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    if (NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 17)
    endif()

    find_package(fep3_participant REQUIRED)
    find_package(fep3_participant_core REQUIRED)
    find_package(fep3_participant_cpp REQUIRED)
    find_package(fep3_components_test REQUIRED)
    enable_testing()
endif ()

option(fep3_participant_cmake_enable_functional_tests
       "Enable functional tests - requires googletest (default: OFF)" OFF)
option(fep3_participant_cmake_enable_private_tests
       "Enable private tests - requires googletest (default: OFF)" OFF)

find_package(Threads)
find_package(GTest)

include(GoogleTest)

if (WIN32 AND CMAKE_CL_64)
    add_definitions(-DWIN64)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (UNIX AND fep3_participant_cmake_integrated_tests AND CMAKE_BUILD_WITH_INSTALL_RPATH)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
endif()

if (fep3_participant_cmake_enable_private_tests)
    # build private tests
    add_subdirectory(private)
endif()

# always build utils of test
add_subdirectory(utils)

if (fep3_participant_cmake_enable_functional_tests)
    add_subdirectory(function)
endif()

if (UNIX AND fep3_participant_cmake_integrated_tests AND CMAKE_BUILD_WITH_INSTALL_RPATH)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
endif()

if (fep3_participant_export_test_targets)
    ##################################################
    # setup a target for the components test library (header-only)
    ##################################################
    add_library(fep3_components_test INTERFACE)

    set(COMPONENTS_TEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/fep3/components)
    target_sources(fep3_components_test INTERFACE
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/base/mock_component.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/base/mock_components.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/clock/mock_clock.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/clock/mock_clock_service.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/clock_sync/mock_clock_sync_service.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/configuration/mock_configuration_service.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/job_registry/mock_job.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/job_registry/mock_job_registry.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/logging/mock_logger.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/logging/mock_logging_service.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/participant_info/mock_participant_info.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/scheduler/mock_scheduler.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/scheduler/mock_scheduler_service.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/service_bus/mock_service_bus.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/simulation_bus/mock_simulation_bus.h>"
        "$<BUILD_INTERFACE:${COMPONENTS_TEST_INCLUDE_DIR}/health_service/mock_health_service.h>"
    )
    target_include_directories(fep3_components_test INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>"
        "$<INSTALL_INTERFACE:test/include/>"
    )
    install(
        TARGETS fep3_components_test
        EXPORT fep3_components_test_targets
        LIBRARY NAMELINK_SKIP DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION lib
    )
    install(EXPORT fep3_components_test_targets DESTINATION lib/cmake)

    # install content from test/include directory
    install(
        DIRECTORY
            include
        DESTINATION
            ./test/
    )

    ###########################################
    # configure the install package
    ###########################################
    # Create version config
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/fep3_components_test-config-version.cmake
        COMPATIBILITY AnyNewerVersion)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fep3_components_test-config-version.cmake DESTINATION .)
    install(FILES fep3_components_test-config.cmake DESTINATION .)
endif()
