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

#include "service_discovery_factory_intf.h"

namespace fep3 {
namespace native {

class ServiceDiscoveryFactory : public IServiceDiscoveryFactory {
public:
    std::unique_ptr<IServiceDiscovery> getServiceDiscovery(
        std::string discover_url,
        std::string network_interface,
        std::chrono::seconds max_age,
        std::pair<std::string, bool> location_url,
        std::string unique_service_name,
        std::string search_target,
        std::string product_name,
        std::string product_version,
        std::string sm_id,
        std::string device_type) const override;

    std::unique_ptr<IServiceFinder> getServiceFinder(
        std::shared_ptr<ILogger> logger_proxy,
        const std::string&,
        const std::string&,
        const std::string&,
        const std::string& search_target,
        const std::string& device_type_filter) const override;
};
} // namespace native
} // namespace fep3
