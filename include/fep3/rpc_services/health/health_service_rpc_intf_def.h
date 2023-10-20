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

#ifndef _FEP3_RPC_HEALTH_DEFINITON_INTF_DEF_H_
#define _FEP3_RPC_HEALTH_DEFINITON_INTF_DEF_H_

// very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3 {
namespace rpc {
namespace catelyn {
/**
 * @brief Definition of the RPC Interface definition of the external service interface of the health
 * service
 * @see health.json file
 */
class IRPCHealthServiceDef {
protected:
    /// DTOR
    ~IRPCHealthServiceDef() = default;

public:
    /// definition of the FEP rpc service iid for the health service interface
    FEP_RPC_IID("health_service.catelyn.fep3.iid", "health_service");
};

} // namespace catelyn
using catelyn::IRPCHealthServiceDef;
} // namespace rpc
} // namespace fep3
#endif // _FEP3_RPC_HEALTH_DEFINITON_INTF_H_
