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

#include "http_client.h"
#include <../3rdparty/lssdp-cpp/src/url/cxx_url.h>

using namespace fep3::arya;

namespace fep3
{
namespace native
{
HttpClientConnector::HttpClientConnector(const std::string& server_address)
{
    fep3::helper::Url url(server_address);
    std::string new_server_address = url.scheme() + "://" + url.host() + ":" + url.port();
    _server_address = new_server_address;
}

HttpClientConnector::~HttpClientConnector()
{
}

fep3::Result HttpClientConnector::sendRequest(const std::string& service_name,
    const std::string& request_message,
    IRPCRequester::IRPCResponse& response_callback) const
{
    try
    {
        ::rpc::http::cJSONClientConnector con(_server_address + "/" + service_name);
        std::string response_message;
        con.SendRPCMessage(request_message, response_message);
        response_callback.set(response_message);
    }
    catch(const std::exception& ex)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
    catch (...)
    {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Unknown error");
    }

    return {};
}
}
}
