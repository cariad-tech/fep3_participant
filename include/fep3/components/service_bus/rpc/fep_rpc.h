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

#pragma once
#pragma warning(push)
#pragma warning(disable : 4996 4290)

#include <fep3/components/service_bus/rpc/fep_json_rpc.h>          //FEP to JSON
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>  //default templates
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_service.h> //default templates
#include <fep3/components/service_bus/rpc/rpc_intf.h>              //public FEP RPC Interface

// include FEP RPC on JSON
#include <rpc/rpc.h>

#pragma warning(pop)
