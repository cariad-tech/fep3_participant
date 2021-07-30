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


set(LIB_DIR "lib")

add_library(test_shared_library SHARED
            test_libraries/test_shared_library.cpp
)
set_target_properties(test_shared_library PROPERTIES
    FOLDER "test/private/shared_library/test_shared_libraries"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LIB_DIR}" # for shared library on linux
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LIB_DIR}" # for dll on windows
)
target_link_libraries(test_shared_library PRIVATE
    fep3_participant
    test_dependee_shared_library
)

add_library(test_dependee_shared_library SHARED
    test_libraries/test_dependee_shared_library.h
    test_libraries/test_dependee_shared_library.cpp
)
target_link_libraries(test_dependee_shared_library PRIVATE fep3_participant)
set_target_properties(test_dependee_shared_library PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LIB_DIR}" # for shared library on linux
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LIB_DIR}" # for dll on windows
    FOLDER "test/private/shared_library/test_shared_libraries"
)

##################################################################
#  the test itself
##################################################################

project(tester_shared_library)

add_executable(${PROJECT_NAME} tester_shared_library.cpp)
add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    TIMEOUT 10
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    GTest::gtest_main
    fep3_participant_private_lib
)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/private/shared_library")

target_compile_definitions(${PROJECT_NAME} PRIVATE
    SHARED_LIBRARY="$<TARGET_FILE:test_shared_library>"
)
