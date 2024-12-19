/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "logger_proxy.h"
#include "service_discovery_intf.h"

#include <utility>

namespace fep3 {
namespace native {

class IServiceDiscoveryFactory {
public:
    virtual ~IServiceDiscoveryFactory(){};
    virtual std::unique_ptr<IServiceDiscovery> getServiceDiscovery(
        std::string discover_url,
        std::string network_interface,
        std::chrono::seconds max_age,
        std::pair<std::string, bool> location_url,
        std::string unique_service_name,
        std::string search_target,
        std::string product_name,
        std::string product_version,
        std::string sm_id = std::string(),
        std::string device_type = std::string()) const = 0;

    virtual std::unique_ptr<IServiceFinder> getServiceFinder(
        std::shared_ptr<ILogger> logger,
        const std::string& discover_url,
        const std::string& product_name,
        const std::string& product_version,
        const std::string& search_target = std::string(),
        const std::string& device_type_filter = std::string()) const = 0;

    IServiceDiscoveryFactory() = default;
    IServiceDiscoveryFactory(IServiceDiscoveryFactory&&) = default;
    IServiceDiscoveryFactory& operator=(IServiceDiscoveryFactory&&) = default;
    IServiceDiscoveryFactory& operator=(const IServiceDiscoveryFactory&) = default;
};

} // namespace native
} // namespace fep3
