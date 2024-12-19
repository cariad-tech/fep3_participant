/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#ifndef _FEP_RPC_HTTP_SERVER_INTF_DEF_H
#define _FEP_RPC_HTTP_SERVER_INTF_DEF_H

// very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3 {
namespace rpc {
namespace catelyn {
/**
 * @brief definition of the external service interface of the service bus
 * @see http_server.json file
 */
class IRPCHttpServerDef {
protected:
    /// DTOR
    ~IRPCHttpServerDef() = default;

public:
    /// definition of the FEP rpc service iid as http_server
    FEP_RPC_IID("http_server.catelyn.fep3.iid", "http_server");
};
} // namespace catelyn
using catelyn::IRPCHttpServerDef;
} // namespace rpc
} // namespace fep3

#endif // _FEP_RPC_HTTP_SERVER_INTF_DEF_H
