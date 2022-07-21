/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */

#pragma once

#include <../3rdparty/lssdp-cpp/src/url/cxx_url.h>
#include <fep3/components/service_bus/system_access_base.hpp>
#include <map>

namespace fep3
{
namespace rti_dds
{

class DDSSystemAccess : public fep3::base::arya::SystemAccessBase
{
    public:
        DDSSystemAccess(const std::string& system_name,
                        const std::string& system_url,
                        const std::shared_ptr<ISystemAccessBaseDefaultUrls>& defaults);
        virtual ~DDSSystemAccess();

    public: //implementation of SystemAccessBase
        std::shared_ptr<IServiceBus::IParticipantServer> createAServer(
            const std::string& server_name,
            const std::string& server_url) override;
        std::shared_ptr<IServiceBus::IParticipantRequester> createARequester(
            const std::string& far_server_name,
            const std::string& far_server_url) const override;
        std::multimap<std::string, std::string> getDiscoveredServices(std::chrono::milliseconds timeout) const override;
        std::multimap<std::string, std::string> getCurrentlyDiscoveredServices() const override;

};

}
}
