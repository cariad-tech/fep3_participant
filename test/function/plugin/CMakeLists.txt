#
# Copyright @ 2022 VW Group. All rights reserved.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

set(TEST_TEMPLATES_DIR "${CMAKE_CURRENT_LIST_DIR}/test_templates")

add_subdirectory(cpp/src)
add_subdirectory(fep_native_components_plugin/src)
add_subdirectory(fep3_http_service_bus_plugin/src)
if(FEP3_USE_RTIDDS)
    add_subdirectory(fep3_connext_dds_plugin/src)
    add_subdirectory(fep3_dds_service_bus_plugin/src)
endif()
