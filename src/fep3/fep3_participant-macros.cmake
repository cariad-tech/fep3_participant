# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

################################################################################
## @page page_cmake_commands
# <hr>
# <b>fep3_participant_install(NAME \<name\> DESTINATION \<destination\>)</b>
#
# This macro installs the target \<name\>, together with the FEP SDK Participant library
# to the folder \<destination\>.
# Arguments:
# @li \<name\>:
# The name of the library to install.
# @li \<destination\>:
# The relative path to the install subdirectory
################################################################################
macro(fep3_participant_install NAME DESTINATION)

    install(TARGETS ${NAME} DESTINATION ${DESTINATION})

    install(
        FILES
            $<TARGET_FILE:fep3_participant>
        DESTINATION ${DESTINATION}
    )

    if(MSVC)
        install(FILES $<TARGET_FILE_DIR:fep3_participant>/fep3_participant$<$<CONFIG:Debug>:d>${fep3_participant_pdb_version_str}.pdb
            DESTINATION ${DESTINATION} OPTIONAL)
    endif()

endmacro(fep3_participant_install NAME DESTINATION)

################################################################################
## @page page_cmake_commands
# <hr>
# <b>fep3_participant_deploy(NAME \<name\>)</b>
#
# This macro deploys the participant library  to the same target folder as the target with \<name\>.
# Arguments:
# @li \<name\>:
# The name of the target to obtain the folder where to copy the participant library
# binaries to.
################################################################################
macro(fep3_participant_deploy NAME)
    fep3_participant_deploy_helper(${NAME}  $<TARGET_FILE_DIR:${NAME}>)
endmacro(fep3_participant_deploy NAME)

################################################################################
## @page page_cmake_commands
# <hr>
# <b>fep3_participant_deploy_test_target(NAME \<TEST_COPY_TARGET\>)</b>
#
# This macro deploys the participant library to the CMAKE_CURRENT_BINARY_DIR folder appended
# with the current configuration name.
# Arguments:
# @li \<name\>:
# The name of the target to obtain the folder where to copy the participant library
# binaries to.
################################################################################
macro(fep3_participant_deploy_test_target TEST_COPY_TARGET)

   # could be that this target is called before any executables or libs and the folder does not exist.
    add_custom_command(TARGET ${TEST_COPY_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
    )

    fep3_participant_deploy_helper(${TEST_COPY_TARGET}  ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
endmacro(fep3_participant_deploy_test_target)

################################################################################
## @page page_cmake_commands
# <hr>
# <b>fep3_participant_create_copy_target(NAME \<name\>)</b>
#
# This macro creates a target named fep_participant_file_copy_${TEST_SUITE}. In case
# a test suite is used that contains multiple executables that copy dlls in the same
# location, then all the executable test targets should depend on the
# fep_participant_file_copy_${TEST_SUITE} and not directly call fep3_participant_deploy_test_target.
# In case of parallel builds this creates problems and is also documented in CMake documentation.
# Arguments:
# @li \<TEST_SUITE\>:
# The name of the test suite that will be appended to the target name
################################################################################
macro(fep3_participant_create_copy_target TEST_SUITE)

    ##################################################################
    # fep_participant_file_copy
    ##################################################################
    add_custom_target(fep_participant_file_copy_${TEST_SUITE})
    fep3_participant_deploy_test_target(fep_participant_file_copy_${TEST_SUITE})

endmacro(fep3_participant_create_copy_target TEST_SUITE)


################################################################################
## @page page_cmake_commands
# <hr>
# <b>internal_fep3_participant_install(NAME \<name\> DESTINATION \<destination\>)</b>
#
# This macro installs the target \<name\>, together with the FEP SDK Participant library and the FEP SDK Participant Plugins.
# to the folder \<destination\>. This macro will removed in the future as FEP plugin deployment becomes available.
# Arguments:
# @li \<name\>:
# The name of the library to install.
# @li \<destination\>:
# The relative path to the install subdirectory
################################################################################
macro(internal_fep3_participant_install NAME DESTINATION)

    fep3_participant_install(${NAME}  ${DESTINATION})

    install(
        FILES $<TARGET_FILE:fep_components_plugin>
        DESTINATION ${DESTINATION}
    )

    # always need a fep_copmonents file
    install(
        FILES $<TARGET_FILE_DIR:fep3_participant>/fep3_participant.fep_components
        DESTINATION ${DESTINATION}
    )

    if(MSVC)
        install(FILES $<TARGET_FILE_DIR:fep_components_plugin>/fep_components_plugin.pdb
                DESTINATION ${DESTINATION} OPTIONAL)
    endif()
    if(fep3_participant_use_rtidds)
        internal_fep3_dds_install(${DESTINATION})

        install(FILES
        $<TARGET_FILE:fep3_connext_dds_plugin>
        $<TARGET_FILE:fep3_dds_service_bus_plugin>
        $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/USER_QOS_PROFILES.xml
        DESTINATION ${DESTINATION}/rti
        )
    endif(fep3_participant_use_rtidds)
endmacro(internal_fep3_participant_install NAME DESTINATION)

################################################################################
## @page page_cmake_commands
# <hr>
# <b>internal_fep3_dds_install(NAME \<name\> DESTINATION \<destination\>)</b>
#
# This macro installs the  dds libraries This macro will removed in the future
# as FEP plugin deployment becomes available.
# Arguments:
# @li \<name\>:
# The name of the library to install.
# @li \<destination\>:
# The relative path to the install subdirectory
################################################################################
macro(internal_fep3_dds_install DESTINATION)
    install(FILES
        $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}rtimonitoring$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
        $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}rticonnextmsgcpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
        $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsmetp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
        $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp2$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
        $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
        $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscore$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
        $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsc$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
        $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/USER_QOS_PROFILES.xml
        DESTINATION ${DESTINATION}/rti
    )

    if(MSVC)
        install(FILES
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/rtimonitoringd.pdb
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/rticonnextmsgcppd.pdb
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddsmetpd.pdb
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscpp2d.pdb
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscppd.pdb
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscored.pdb
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscd.pdb
            DESTINATION ${DESTINATION}/rti
            CONFIGURATIONS Debug
        )
    endif()
endmacro(internal_fep3_dds_install NAME DESTINATION)

################################################################################
## @page page_cmake_commands
# <hr>
# <b>fep3_participant_deploy_helper(NAME \<name\> TARGET_FOLDER \<target_folder\>)</b>
#
# This macro deploys the participant library to the \<target_folder\>.
# Arguments:
# @li \<name\>:
# The name of the target to obtain the folder where to copy the participant library
# binaries to.
# @li \<target_folder\>:
# The path where to copy the participant library
# binaries to.
################################################################################
macro(fep3_participant_deploy_helper NAME TARGET_FOLDER)
    # no need to copy in build directory on linux since linker rpath takes care of that
    if (WIN32)
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep3_participant> ${TARGET_FOLDER}
        )

    endif()

    if(MSVC)
        add_custom_command(TARGET ${NAME} POST_BUILD
            # command true is a no op in CMAKE, and executed in case we are in release build
            COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Debug>>,copy_if_different,true>
            $<TARGET_FILE_DIR:fep3_participant>/fep3_participant$<$<CONFIG:Debug>:d>${fep3_participant_pdb_version_str}.pdb
            ${TARGET_FOLDER})
    endif()

    set_target_properties(${NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")
endmacro(fep3_participant_deploy_helper NAME)

################################################################################
## @page page_cmake_commands
# <hr>
# <b>internal_fep3_participant_deploy_helper(NAME \<name\> TARGET_FOLDER \<target_folder\>)</b>
#
# This macro deploys the participant library and plugins to the \<target_folder\>. This macro
# will removed in the future as FEP plugin deployment becomes available.
# Arguments:
# @li \<name\>:
# The name of the target to obtain the folder where to copy the participant library
# binaries to.
# @li \<target_folder\>:
# The path where to copy the participant library
# binaries to.
################################################################################
macro(internal_fep3_participant_deploy_helper NAME TARGET_FOLDER)
    fep3_participant_deploy_helper(${NAME} ${TARGET_FOLDER})

    #native component plugin is loaded and not using rpath, so it has to be copied always.
    add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fep_components_plugin> ${TARGET_FOLDER}
    )
    # always need a fep_copmponents file for the native components plugin
    add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:fep3_participant>/fep3_participant.fep_components
        ${TARGET_FOLDER}
    )

    if(MSVC)
        add_custom_command(TARGET ${NAME} POST_BUILD
        # command true is a no op in CMAKE, and executed in case we are in release build
            COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Debug>>,copy_if_different,true>
            $<TARGET_FILE_DIR:fep_components_plugin>/fep_components_plugin.pdb
            ${TARGET_FOLDER})
    endif()

    if(fep3_participant_use_rtidds)
        internal_fep3_participant_deploy_dds(${NAME} ${TARGET_FOLDER})

        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:fep3_connext_dds_plugin>
                $<TARGET_FILE:fep3_dds_service_bus_plugin>
                ${TARGET_FOLDER}/rti
        )

        if(MSVC)
            add_custom_command(TARGET ${NAME} POST_BUILD
            # command true is a no op in CMAKE, and executed in case we are in release build
                COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:RelWithDebInfo>,$<CONFIG:Debug>>,copy_if_different,true>
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/fep3_connext_dds_plugin.pdb
                $<TARGET_FILE_DIR:fep3_dds_service_bus_plugin>/fep3_dds_service_bus_plugin.pdb
                ${TARGET_FOLDER}/rti
            )
        endif()

        #now we need to install the components file to load the rti plugin
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE_DIR:fep3_participant>/fep3_participant.fep_components
            ${TARGET_FOLDER}
        )
    endif(fep3_participant_use_rtidds)
endmacro(internal_fep3_participant_deploy_helper NAME)

################################################################################
## @page page_cmake_commands
# <hr>
# <b>internal_fep3_participant_deploy_dds(NAME \<name\> TARGET_FOLDER \<target_folder\>)</b>
#
# This macro deploys the DDS libraris to the \<target_folder\>. This macro
# will removed in the future as FEP plugin deployment becomes available.
# Arguments:
# @li \<name\>:
# The name of the target to obtain the folder where to copy the participant library
# binaries to.
# @li \<target_folder\>:
# The path where to copy the participant library
# binaries to.
################################################################################
macro(internal_fep3_participant_deploy_dds NAME TARGET_FOLDER)

    add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
                 ${TARGET_FOLDER}/rti
    )
    add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}rtimonitoring$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}rticonnextmsgcpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsmetp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp2$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscore$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsc$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/USER_QOS_PROFILES.xml
            ${TARGET_FOLDER}/rti
    )

    if(MSVC)
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/rtimonitoring$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/rticonnextmsgcpp$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddsmetp$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscpp2$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscpp$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddscore$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/nddsc$<$<CONFIG:Debug>:d>.$<$<CONFIG:Debug>:pdb>$<$<CONFIG:Release>:dll>$<$<CONFIG:RelWithDebInfo>:dll>
                ${TARGET_FOLDER}/rti
        )
    endif()

endmacro(internal_fep3_participant_deploy_dds NAME)

################################################################################
## @page page_cmake_commands
# <hr>
# <b>internal_fep3_participant_deploy(NAME \<name\>)</b>
#
# This macro deploys the participant library and plugins to the same target folder as the target with \<name\>.
# This macro will removed in the future as FEP plugin deployment becomes available.
# Arguments:
# @li \<name\>:
# The name of the target to obtain the folder where to copy the participant library
# binaries to.
################################################################################
macro(internal_fep3_participant_deploy NAME)
    internal_fep3_participant_deploy_helper(${NAME}  $<TARGET_FILE_DIR:${NAME}>)
endmacro(internal_fep3_participant_deploy NAME)

################################################################################
## @page page_cmake_commands
# <hr>
# <b>internal_fep3_participant_deploy_test_target(NAME \<TEST_COPY_TARGET\>)</b>
#
# This macro deploys the participant library and plugins to the CMAKE_CURRENT_BINARY_DIR folder appended
# with the current configuration name. This macro will removed in the future as FEP plugin
# deployment becomes available.
# Arguments:
# @li \<name\>:
# The name of the target to obtain the folder where to copy the participant library
# binaries to.
################################################################################
macro(internal_fep3_participant_deploy_test_target TEST_COPY_TARGET)

    #could be that this target is called before any executables or libs and the folder does not exist.
    add_custom_command(TARGET ${TEST_COPY_TARGET} POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>"
    )

    internal_fep3_participant_deploy_helper(${TEST_COPY_TARGET}  ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
endmacro(internal_fep3_participant_deploy_test_target)

################################################################################
## @page page_cmake_commands
# <hr>
# <b>internal_fep3_participant_create_copy_target(NAME \<name\>)</b>
#
# This macro creates a target named fep_participant_file_copy_${TEST_SUITE}. In case
# a test suite is used that contains multiple executables that copy dlls in the same
# location, then all the executable test targets should depend on the
# fep_participant_file_copy_${TEST_SUITE} and not directly call internal_fep3_participant_deploy_test_target.
# In case of parallel builds this creates problems and is also documented in CMake documentation.
# This macro will be removed in the future as FEP plugin deployment becomes available.
# Arguments:
# @li \<TEST_SUITE\>:
# The name of the test suite that will be appended to the target name
################################################################################
macro(internal_fep3_participant_create_copy_target TEST_SUITE)

    ##################################################################
    # fep_participant_file_copy
    ##################################################################
    add_custom_target(fep_participant_file_copy_${TEST_SUITE})
    internal_fep3_participant_deploy_test_target(fep_participant_file_copy_${TEST_SUITE})

endmacro(internal_fep3_participant_create_copy_target TEST_SUITE)
################################################################################
## @page page_cmake_commands
# <hr>
# <b>fep3_participant_link_pthread(TARGET_NAME \<target_name\>)</b>
#
# This macro links pthread to \<target_name\>, in order to avoid
# https://sourceware.org/bugzilla/show_bug.cgi?id=16628,
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67791
# Arguments:
# @li \<target_name\>:
# The name of the target to link with pthread.
################################################################################
macro(fep3_participant_link_pthread target_name)
    if (UNIX)
        set(THREADS_PREFER_PTHREAD_FLAG TRUE)
        find_package(Threads REQUIRED)
        target_link_libraries(${target_name} PUBLIC Threads::Threads)
    endif (UNIX)
endmacro(fep3_participant_link_pthread)

function(fep_generate_rpc_stubs_before_target)
    # check necessary dependencies
    if(NOT TARGET dev_essential)
        find_package(dev_essential 1.3.0 REQUIRED COMPONENTS jsonrpcstub)
    endif()

    # get all passed argument values
    set(one_value_args TARGET
                       INPUT_FILE         # must be absolute
                       OUTPUT_DIR         # must be absolute
                       CLIENT_CLASS_NAME
                       SERVER_CLASS_NAME
                       CLIENT_FILE_NAME   # relative to OUTPUT_DIR
                       SERVER_FILE_NAME)  # relative to OUTPUT_DIR
    cmake_parse_arguments(PARSE_ARGV 0 function_arg "" "${one_value_args}" "")

    # necessary path checks and normalizations
    cmake_path(IS_RELATIVE function_arg_INPUT_FILE _is_relative)
    if(_is_relative)
        message(FATAL_ERROR "INPUT_FILE must be absolute path.")
    endif()

    cmake_path(IS_RELATIVE function_arg_OUTPUT_DIR _is_relative)
    if(_is_relative)
        message(FATAL_ERROR "OUTPUT_DIR must be absolute path.")
    endif()

    cmake_path(CONVERT "${function_arg_INPUT_FILE}" TO_CMAKE_PATH_LIST _input_file NORMALIZE)
    cmake_path(CONVERT "${function_arg_OUTPUT_DIR}" TO_CMAKE_PATH_LIST _output_dir NORMALIZE)

    message(STATUS "Target '${function_arg_TARGET}' will generate the following rpc stub(s) from json file: ${_input_file}")
    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    # command for client stub
    if(DEFINED function_arg_CLIENT_CLASS_NAME AND DEFINED function_arg_CLIENT_FILE_NAME)
        cmake_path(APPEND _output_dir "${function_arg_CLIENT_FILE_NAME}" OUTPUT_VARIABLE _client_output_file)
        set(_create_client_stub $<TARGET_FILE:dev_essential::jsonrpcstub> "${_input_file}"
                                --cpp-client=${function_arg_CLIENT_CLASS_NAME}
                                --cpp-client-file=${_client_output_file})
        list(APPEND _output_files "${_client_output_file}")
        message(STATUS "Client stub: ${_client_output_file}")
    endif()

    # command for server stub
    if(DEFINED function_arg_SERVER_CLASS_NAME AND DEFINED function_arg_SERVER_FILE_NAME)
        cmake_path(APPEND _output_dir "${function_arg_SERVER_FILE_NAME}" OUTPUT_VARIABLE _server_output_file)
        set(_create_server_stub $<TARGET_FILE:dev_essential::jsonrpcstub> "${_input_file}"
                                --cpp-server=${function_arg_SERVER_CLASS_NAME}
                                --cpp-server-file=${_server_output_file})
        list(APPEND _output_files "${_server_output_file}")
        message(STATUS "Server stub: ${_server_output_file}")
    endif()

    if (NOT (DEFINED _create_client_stub OR DEFINED _create_server_stub))
        message(FATAL_ERROR "Either server or client arguments are incorrect.")
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endif()

    # jsonrpc_generate_stub command silently fails if the output directory does not exist...
    # generate it at built time, if it doesn't exist
    set(_mkdir_output "${CMAKE_COMMAND}" -E make_directory "${_output_dir}")

    # add files to target's sources and add dependend custom command
    set_target_properties(${function_arg_TARGET} PROPERTIES FOLDER rpc_stub_generators)
    target_sources(${function_arg_TARGET} PRIVATE "${_server_output_file}" "${_client_output_file}" "${_input_file}")

    add_custom_command(OUTPUT ${_output_files}
                       COMMAND ${_mkdir_output}
                       COMMAND ${_create_client_stub}
                       COMMAND ${_create_server_stub}
                       MAIN_DEPENDENCY "${_input_file}"
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                       VERBATIM)
    list(POP_BACK CMAKE_MESSAGE_INDENT)
endfunction()
