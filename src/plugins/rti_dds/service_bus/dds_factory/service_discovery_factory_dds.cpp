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

#include "service_discovery_factory_dds.h"

#include "dds_service_discovery.h"
#include "http_systemaccess.h"
#include "service_discovery.h"

namespace fep3 {
namespace native {

std::unique_ptr<IServiceDiscovery> ServiceDiscoveryFactory::getServiceDiscovery(
    std::string,
    std::string,
    std::chrono::seconds,
    std::pair<std::string, bool> location_url,
    std::string unique_service_name,
    std::string,
    std::string,
    std::string,
    std::string,
    std::string) const
{
    return std::make_unique<ServiceDiscovery<ServiceDiscoveryDDS>>(
        std::move(location_url), std::move(unique_service_name), _dds_sd_participant);
}

std::unique_ptr<IServiceFinder> ServiceDiscoveryFactory::getServiceFinder(
    std::shared_ptr<ILogger> logger,
    const std::string& discover_url,
    const std::string&,
    const std::string&,
    const std::string&,
    const std::string&) const
{
    if (discover_url != fep3::native::HttpSystemAccess::_default_url) {
        using namespace std::literals::string_literals;
        logger->logError("User defined Server Url"s + discover_url +
                         " is ignored in service bus dds discovery");
    }
    return std::make_unique<ServiceFinder<ServiceFinderDDS>>(std::move(logger),
                                                             _dds_sd_participant);
}

} // namespace native
} // namespace fep3
