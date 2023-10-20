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

#include "http_client.h"

#include <a_util/system/system.h>
#include <rpc/http/json_http_rpc.h>

#include <algorithm>
#include <ctype.h>
#include <cxx_url.h>

using namespace fep3::arya;

namespace fep3 {
namespace native {

class JSONClientConnectorCache {
public:
    ::rpc::http::cJSONClientConnector& getConnector(const std::string& server_address,
                                                    const std::string& service_name)
    {
        const std::string url = server_address + "/" + service_name;
        if (_cached_connectors.count(url) == 0) {
            _cached_connectors.emplace(url, ::rpc::http::cJSONClientConnector(url));
        }

        return _cached_connectors.at(url);
    }

private:
    std::map<std::string, ::rpc::http::cJSONClientConnector> _cached_connectors;
};

HttpClientConnector::HttpClientConnector(const std::string& server_address)
    : _json_connector_cache(std::make_unique<JSONClientConnectorCache>())
{
    setServerAddress(server_address);
}

void HttpClientConnector::setServerAddress(const std::string& server_address)
{
    fep3::helper::Url url(server_address);

    auto string_to_upper = [](const std::string& input) -> std::string {
        std::string input_upper = input;
        std::transform(input_upper.begin(),
                       input_upper.end(),
                       input_upper.begin(),
                       [](unsigned char c) { return static_cast<char>(toupper(c)); });

        return input_upper;
    };

    std::string local_host_name_upper = string_to_upper(a_util::system::getHostname());
    std::string host_name_upper = string_to_upper(url.host());

    std::string url_host_new;

    // in case the host name server_address is the same as the participant, then we know the ip is
    // 127.0.0.1
    if (host_name_upper == local_host_name_upper) {
        url_host_new = "127.0.0.1";
    }
    else {
        url_host_new = url.host();
    }

    std::string new_server_address = url.scheme() + "://" + url_host_new + ":" + url.port();
    _server_address = new_server_address;
}

HttpClientConnector::~HttpClientConnector()
{
}

fep3::Result HttpClientConnector::sendRequest(const std::string& service_name,
                                              const std::string& request_message,
                                              IRPCRequester::IRPCResponse& response_callback) const
{
    try {
        ::rpc::http::cJSONClientConnector& con =
            _json_connector_cache->getConnector(_server_address, service_name);
        std::string response_message;
        con.SendRPCMessage(request_message, response_message);
        response_callback.set(response_message);
    }
    catch (const std::exception& ex) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, ex.what());
    }
    catch (...) {
        RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Unknown error");
    }

    return {};
}
} // namespace native
} // namespace fep3
