#
# Copyright @ 2021 VW Group. All rights reserved.
#
#     This Source Code Form is subject to the terms of the Mozilla
#     Public License, v. 2.0. If a copy of the MPL was not distributed
#     with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# If it is not possible or desirable to put the notice in a particular file, then
# You may include the notice in a location (such as a LICENSE file in a
# relevant directory) where a recipient would be likely to look for such a notice.
#
# You may add additional accurate notices of copyright ownership.
#
#


################################################################################
## \page page_cmake_commands
# <hr>
# <b>fep3_participant_install(NAME \<name\> DESTINATION \<destination\>)</b>
#
# This macro installs the target \<name\>, together with the FEP SDK Participant libraries (if neccessary)
#   to the folder \<destination\>
# Arguments:
# \li \<name\>:
# The name of the library to install.
# \li \<destination\>:
# The relative path to the install subdirectory
################################################################################
macro(fep3_participant_install NAME DESTINATION)
    install(TARGETS ${NAME} DESTINATION ${DESTINATION})
    install(
        FILES
            $<TARGET_FILE:fep3_participant>
        DESTINATION ${DESTINATION}
    )

    install(
        FILES
            $<TARGET_FILE:fep_components_plugin>
        DESTINATION ${DESTINATION}
    )

     # always need a fep_copmonents file
        install(
            FILES
                $<TARGET_FILE_DIR:fep3_participant>/fep3_participant.fep_components
            DESTINATION ${DESTINATION}
        )

    if(MSVC)
        install(FILES $<TARGET_FILE_DIR:fep3_participant>/fep3_participant$<$<CONFIG:Debug>:d>${fep3_participant_pdb_version_str}.pdb
                DESTINATION ${DESTINATION} OPTIONAL)

        install(FILES $<TARGET_FILE_DIR:fep_components_plugin>/fep_components_plugin.pdb
                DESTINATION ${DESTINATION} OPTIONAL)
    endif(WIN32)

    if(fep3_participant_use_rtidds)
        install(FILES
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}rtimonitoring$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}rticonnextmsgcpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsmetp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp2$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscpp$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddscore$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE_DIR:fep3_connext_dds_plugin>/${CMAKE_SHARED_LIBRARY_PREFIX}nddsc$<$<CONFIG:Debug>:d>${CMAKE_SHARED_LIBRARY_SUFFIX}
            $<TARGET_FILE:fep3_connext_dds_plugin>
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

    endif()
endmacro(fep3_participant_install NAME DESTINATION)

################################################################################
## \page page_cmake_commands
# <hr>
# <b>fep3_participant_deploy_helper(NAME \<name\> TARGET_FOLDER \<target_folder\>)</b>
#
# This macro deploys the participant library to the \<target_folder\>.
# Arguments:
# \li \<name\>:
# The name of the target to obtain the folder where to copy the participant library
# binaries to.
# \li \<target_folder\>:
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
        $<TARGET_FILE_DIR:fep3_participant>/fep3_participant$<$<CONFIG:Debug>:d>${fep3_participant_pdb_version_str}.pdb
        ${TARGET_FOLDER})
    endif()
        
    if(fep3_participant_use_rtidds)

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
                $<TARGET_FILE:fep3_connext_dds_plugin>
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

        #now we need to install the components file to load the rti plugin
        add_custom_command(TARGET ${NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE_DIR:fep3_participant>/fep3_participant.fep_components
                ${TARGET_FOLDER}
        )
    endif()
    set_target_properties(${NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")
endmacro(fep3_participant_deploy_helper NAME)

################################################################################
## \page page_cmake_commands
# <hr>
# <b>fep3_participant_deploy(NAME \<name\>)</b>
#
# This macro deploys the participant library to the same target folder as the target with \<name\>.
# Arguments:
# \li \<name\>:
# The name of the target to obtain the folder where to copy the participant library
# binaries to.
################################################################################
macro(fep3_participant_deploy NAME)
    fep3_participant_deploy_helper(${NAME}  $<TARGET_FILE_DIR:${NAME}>)
endmacro(fep3_participant_deploy NAME)

################################################################################
## \page page_cmake_commands
# <hr>
# <b>fep3_participant_deploy_test_target(NAME \<TEST_COPY_TARGET\>)</b>
#
# This macro deploys the participant library to the CMAKE_CURRENT_BINARY_DIR folder appended
# with the current configuration name
# Arguments:
# \li \<name\>:
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
## \page page_cmake_commands
# <hr>
# <b>fep3_participant_create_copy_target(NAME \<name\>)</b>
#
# This macro creates a target named fep_participant_file_copy_${TEST_SUITE}. In case
# a test suite is used that contains multiple executables that copy dlls in the same
# location, then all the executable test targets should depend on the
# fep_participant_file_copy_${TEST_SUITE} and not directly call fep3_participant_deploy_test_target.
# In case of parallel builds this creates problems and is also documented in CMake documentation.
# Arguments:
# \li \<TEST_SUITE\>:
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
## \page page_cmake_commands
# <hr>
# <b>fep3_participant_link_pthread(TARGET_NAME \<target_name\>)</b>
#
# This macro links pthread to \<target_name\>, in order to avoid
# https://sourceware.org/bugzilla/show_bug.cgi?id=16628, 
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67791
# Arguments:
# \li \<target_name\>:
# The name of the target to link with pthread.
################################################################################
macro(fep3_participant_link_pthread target_name)
	if (UNIX)
		set(THREADS_PREFER_PTHREAD_FLAG TRUE)
		find_package(Threads REQUIRED)
		target_link_libraries(${target_name} PUBLIC Threads::Threads)
	endif (UNIX)
endmacro(fep3_participant_link_pthread)

