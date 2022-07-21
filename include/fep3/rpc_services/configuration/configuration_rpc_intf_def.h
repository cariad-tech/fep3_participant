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


#ifndef _FEP3_RPC_CONFIGURATION_DEFINITON_INTF_DEF_H_
#define _FEP3_RPC_CONFIGURATION_DEFINITON_INTF_DEF_H_

#include <vector>
#include <string>
//very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3
{
namespace rpc
{
namespace arya
{
    /**
     * @brief Definition of the RPC Interface definition of the external service interface of the configuration
     * @see configuration.json file
     */
    class IRPCConfigurationDef
    {
        protected:
            /// DTOR
            ~IRPCConfigurationDef() = default;

        public:
            ///definition of the FEP rpc service iid for the configuration interface
            FEP_RPC_IID("configuration_service.arya.fep3.iid", "configuration");
    };
} // namespace arya
using arya::IRPCConfigurationDef;
} // namespace rpc
} // namespace fep3

#endif // _FEP3_RPC_CONFIGURATION_DEFINITON_INTF_H_
