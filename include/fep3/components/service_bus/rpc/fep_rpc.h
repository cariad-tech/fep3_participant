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

#pragma once
#pragma warning(push)
#pragma warning(disable : 4996 4290)

#include "rpc_intf.h"   //public FEP RPC Interface

//include FEP RPC on JSON
#include <rpc/rpc.h>
#include "fep_json_rpc.h"   //FEP to JSON
#include "fep_rpc_stubs_client.h"  //default templates
#include "fep_rpc_stubs_service.h"  //default templates
#pragma warning(pop)

