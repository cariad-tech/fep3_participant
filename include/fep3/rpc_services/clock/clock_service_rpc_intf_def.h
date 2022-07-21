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


#ifndef _FEP3_RPC_CLOCK_SERVICE_INTF_DEF_H_
#define _FEP3_RPC_CLOCK_SERVICE_INTF_DEF_H_

//very important to have this relative! system library!
#include "../base/fep_rpc_iid.h"

namespace fep3
{
namespace rpc
{
namespace arya
{

/**
 * @brief definition of the external service interface of the clock service
 * @see delivered clock_service.json file
 */
class IRPCClockServiceDef
{
protected:
    /// DTOR
    ~IRPCClockServiceDef() = default;

public:
    ///definition of the FEP rpc service iid for the clock service
    FEP_RPC_IID("clock_service.arya.fep3.iid", "clock_service");
};

} // namespace arya
using arya::IRPCClockServiceDef;
} // namespace rpc
} // namespace fep3

#endif // _FEP3_RPC_CLOCK_SERVICE_INTF_H_
