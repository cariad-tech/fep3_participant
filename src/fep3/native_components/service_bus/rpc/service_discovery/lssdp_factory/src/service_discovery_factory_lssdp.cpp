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

#include "service_discovery_factory_lssdp.h"

#include "service_discovery.h"

namespace fep3 {
namespace native {

std::unique_ptr<IServiceDiscovery> ServiceDiscoveryFactory::getServiceDiscovery(
    std::string discover_url,
    std::string network_interface,
    std::chrono::seconds max_age,
    std::pair<std::string, bool> location_url,
    std::string unique_service_name,
    std::string search_target,
    std::string product_name,
    std::string product_version,
    std::string sm_id,
    std::string device_type) const
{
    return std::make_unique<ServiceDiscovery<lssdp::Service>>(std::move(discover_url),
                                                              std::move(network_interface),
                                                              std::move(max_age),
                                                              std::move(location_url),
                                                              std::move(unique_service_name),
                                                              std::move(search_target),
                                                              std::move(product_name),
                                                              std::move(product_version),
                                                              std::move(sm_id),
                                                              std::move(device_type));
}

std::unique_ptr<IServiceFinder> ServiceDiscoveryFactory::getServiceFinder(
    std::shared_ptr<ILogger>,
    const std::string& discover_url,
    const std::string& product_name,
    const std::string& product_version,
    const std::string& search_target,
    const std::string& device_type_filter) const
{
    return std::make_unique<ServiceFinder<lssdp::ServiceFinder>>(std::move(discover_url),
                                                                 std::move(product_name),
                                                                 std::move(product_version),
                                                                 std::move(search_target),
                                                                 std::move(device_type_filter));
}

} // namespace native
} // namespace fep3
