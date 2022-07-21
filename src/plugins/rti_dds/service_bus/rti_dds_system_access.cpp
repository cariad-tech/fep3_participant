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

#include "rti_dds_system_access.h"
#include <a_util/result.h>

using namespace fep3::arya;

namespace fep3
{
namespace rti_dds
{
/*******************************************************************************************
 *
 *******************************************************************************************/
/*
struct RPCResponseToFEPResponse : public IRPCRequester::IRPCResponse
{
    rpc::IResponse& _bounded_response;
    RPCResponseToFEPResponse(rpc::IResponse& response_to_bind) : _bounded_response(response_to_bind)
    {
    }
    fep3::Result set(const std::string& response)
    {
        _bounded_response.Set(response.c_str(), response.size());
        return {};
    }
};*/

/*******************************************************************************************
 *
 *******************************************************************************************/



/*******************************************************************************************
 *
 *******************************************************************************************/

DDSSystemAccess::DDSSystemAccess(const std::string& system_name,
    const std::string& system_url,
    const std::shared_ptr<ISystemAccessBaseDefaultUrls>& defaults)
    : fep3::base::arya::SystemAccessBase(system_name, system_url, defaults)
{
}
DDSSystemAccess::~DDSSystemAccess()
{
}

std::shared_ptr<IServiceBus::IParticipantServer> DDSSystemAccess::createAServer(
    const std::string& ,
    const std::string& )
{
    return {};
}

std::shared_ptr<IServiceBus::IParticipantRequester> DDSSystemAccess::createARequester(
    const std::string& ,
    const std::string& ) const
{
    return {};
}

std::multimap<std::string, std::string> DDSSystemAccess::getDiscoveredServices(std::chrono::milliseconds) const
{
    return {};
}

std::multimap<std::string, std::string> DDSSystemAccess::getCurrentlyDiscoveredServices() const
{
    return {};
}

}
}

