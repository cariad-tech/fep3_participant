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

#ifndef _FEP3_RPC_JOB_REGISTRY_INTF_DEF_H_
#define _FEP3_RPC_JOB_REGISTRY_INTF_DEF_H_

// very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3 {
namespace rpc {
namespace catelyn {

/**
 * @brief definition of the external service interface of the job registry
 * @see delivered job_registry.json file
 */
class IRPCJobRegistryDef {
protected:
    /// DTOR
    ~IRPCJobRegistryDef() = default;

public:
    /// definition of the FEP rpc service iid for the job registry
    FEP_RPC_IID("job_registry.catelyn.fep3.iid", "job_registry");
};

} // namespace catelyn
using catelyn::IRPCJobRegistryDef;
} // namespace rpc
} // namespace fep3

#endif // _FEP3_RPC_JOB_REGISTRY_INTF_DEF_H_
