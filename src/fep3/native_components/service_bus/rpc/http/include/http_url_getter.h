/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include "system_access_base.hpp"

namespace fep3 {
namespace helper {

std::pair<fep3::Result, std::string> getSystemUrl(
    const std::string& input_url,
    const fep3::base::SystemAccessBase::ISystemAccessBaseDefaultUrls& service_bus_system_default);

std::string getServerUrl(
    const std::string& input_url,
    const fep3::base::SystemAccessBase::ISystemAccessBaseDefaultUrls& service_bus_system_default);
} // namespace helper
} // namespace fep3
