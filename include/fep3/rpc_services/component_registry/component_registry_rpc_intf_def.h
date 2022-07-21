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


#ifndef _FEP_RPC_COMPONENT_REGISTRY_INTF_DEF_H
#define _FEP_RPC_COMPONENT_REGISTRY_INTF_DEF_H

#include <vector>
#include <string>
// very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3 {
namespace rpc {

namespace bronn {
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
    FEP_RPC_IID("component_registry.bronn.rpc.fep3.iid", "component_registry");
};

} // namespace bronn
} // namespace rpc
} // namespace fep3

#endif // _FEP_RPC_COMPONENT_REGISTRY_INTF_DEF_H
