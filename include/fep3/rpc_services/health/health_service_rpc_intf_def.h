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


#ifndef _FEP3_RPC_HEALTH_DEFINITON_INTF_DEF_H_
#define _FEP3_RPC_HEALTH_DEFINITON_INTF_DEF_H_

#include <string>
//very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3
{
namespace experimental
{
    /**
     * @brief Definition of the RPC Interface definition of the external service interface of the health service
     * @see health.json file
     */
    class IRPCHealthServiceDef
    {
        protected:
            /// DTOR
            ~IRPCHealthServiceDef() = default;

        public:
            ///definiton of the FEP rpc service iid for the health service interface
            FEP_RPC_IID("health_service.experimental.fep3.iid", "health_service");
    };
//using arya::IRPCHealthServiceDef;
} // namespace experimental
} // namespace fep3

#endif // _FEP3_RPC_HEALTH_DEFINITON_INTF_H_
