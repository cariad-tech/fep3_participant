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

#ifndef _FEP3_RPC_CONFIGURATION_DEFINITON_INTF_DEF_H_
#define _FEP3_RPC_CONFIGURATION_DEFINITON_INTF_DEF_H_

// very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3 {
namespace rpc {
namespace arya {
/**
 * @brief Definition of the RPC Interface definition of the external service interface of the
 * configuration
 * @see configuration.json file
 */
class IRPCConfigurationDef {
protected:
    /// DTOR
    ~IRPCConfigurationDef() = default;

public:
    /// definition of the FEP rpc service iid for the configuration interface
    FEP_RPC_IID("configuration_service.arya.fep3.iid", "configuration");
};
} // namespace arya
using arya::IRPCConfigurationDef;
} // namespace rpc
} // namespace fep3

#endif // _FEP3_RPC_CONFIGURATION_DEFINITON_INTF_H_
