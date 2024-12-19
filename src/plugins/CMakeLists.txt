# Copyright 2023 CARIAD SE.
#
# This Source Code Form is subject to the terms of the Mozilla
# Public License, v. 2.0. If a copy of the MPL was not distributed
# with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

if (fep3_participant_use_rtidds)
    add_subdirectory(rti_dds)
endif()

add_subdirectory(http)
add_subdirectory(fep_native_components)

