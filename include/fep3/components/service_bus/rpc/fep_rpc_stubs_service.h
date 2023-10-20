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

#include <fep3/components/service_bus/rpc/fep_json_rpc.h>

#include <rpc/json_rpc.h>

namespace fep3 {
namespace rpc {
namespace arya {
/**
 * @brief Parent class of all rpc servers.
 * @tparam ServerStub Generated server stub.
 * @tparam Interface Server id interface.
 */
template <typename server_stub, typename rpc_interface>
class RPCService
    : protected ::rpc::jsonrpc_object_server<server_stub, arya::detail::JSONFEPServerConnector>,
      public fep3::arya::IRPCServer::IRPCService {
protected:
    /// definiton of base class
    typedef ::rpc::jsonrpc_object_server<server_stub, arya::detail::JSONFEPServerConnector>
        _base_class;

public: // implements IRPCServer::IRPCService
    // @cond no_documentation
    fep3::Result handleRequest(const std::string& /*content_type*/,
                               const std::string& request_message,
                               fep3::arya::IRPCRequester::IRPCResponse& response_message) override
    {
        try {
            arya::detail::FEPResponseToRPCResponse local_response_wrap_up(response_message);
            if (!_base_class::OnRequest(request_message, &local_response_wrap_up)) {
                return fep3::Result(fep3::ERR_INVALID_ARG,
                                    "invalid argument in rpc OnRequest call",
                                    __LINE__,
                                    __FILE__,
                                    "handleRequest");
            }
        }
        catch (std::exception& exce) {
            return fep3::Result(
                fep3::ERR_EXCEPTION_RAISED, exce.what(), __LINE__, __FILE__, "handleRequest");
        }

        return fep3::Result();
    }
    // @endcond no_documentation

    //! @copydoc fep3::IRPCServer::IRPCService::getRPCServiceIIDs
    std::string getRPCServiceIIDs() const override
    {
        return rpc_interface::RPC_IID;
    }

    //! @copydoc fep3::IRPCServer::IRPCService::getRPCInterfaceDefinition
    std::string getRPCInterfaceDefinition() const override
    {
        return server_stub::interface_definition;
    }

private:
    std::string _service_name;
};

} // namespace arya
using arya::RPCService;
} // namespace rpc
} // namespace fep3
