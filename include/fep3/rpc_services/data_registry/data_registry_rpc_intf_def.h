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

#ifndef _FEP3_RPC_DATAREGISTRY_INTF_DEF_H_
#define _FEP3_RPC_DATAREGISTRY_INTF_DEF_H_

// very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3 {
namespace rpc {
namespace catelyn {
/**
 * @brief definition of the external service interface of the data registry
 * @see data_registry.json file
 */
class IRPCDataRegistryDef {
protected:
    /// DTOR
    ~IRPCDataRegistryDef() = default;

public:
    /// definition of the FEP rpc service iid of the data registry
    FEP_RPC_IID("data_registry.catelyn.fep3.iid", "data_registry");
};
} // namespace catelyn
using catelyn::IRPCDataRegistryDef;
} // namespace rpc
} // namespace fep3

#endif //_FEP3_RPC_DATAREGISTRY_INTF_DEF_H_
