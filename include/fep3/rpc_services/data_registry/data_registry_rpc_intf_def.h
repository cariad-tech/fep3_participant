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

#ifndef _FEP3_RPC_DATAREGISTRY_INTF_DEF_H_
#define _FEP3_RPC_DATAREGISTRY_INTF_DEF_H_

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
* @brief definition of the external service interface of the data registry
* @see data_registry.json file
*/
class IRPCDataRegistryDef
{
protected:
    /// DTOR
    ~IRPCDataRegistryDef() = default;

public:
    ///definition of the FEP rpc service iid of the data registry
    FEP_RPC_IID("data_registry.arya.fep3.iid", "data_registry");

};
} // namespace arya
using arya::IRPCDataRegistryDef;
} // namespace rpc
} // namespace fep3

#endif //_FEP3_RPC_DATAREGISTRY_INTF_DEF_H_