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

#ifndef _FEP_RPC_COMPONENT_REGISTRY_INTF_DEF_H
#define _FEP_RPC_COMPONENT_REGISTRY_INTF_DEF_H

// very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3 {
namespace rpc {

namespace catelyn {
/**
 * @brief definition of the external service interface of the participant itself
 * @see COMPONENT_REGISTRY.json file
 */
class IRPCComponentRegistryDef {
protected:
    /// DTOR
    ~IRPCComponentRegistryDef() = default;

public:
    /// definition of the FEP rpc service iid as component_registry service
    FEP_RPC_IID("component_registry.catelyn.rpc.fep3.iid", "component_registry");
};

} // namespace catelyn
using catelyn::IRPCComponentRegistryDef;
} // namespace rpc
} // namespace fep3

#endif // _FEP_RPC_COMPONENT_REGISTRY_INTF_DEF_H
