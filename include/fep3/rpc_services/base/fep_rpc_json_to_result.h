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
#pragma once

#include <fep3/fep3_result_decl.h>

#include <jsonrpccpp/client.h>

///@cond nodoc
namespace fep3::rpc::arya {

inline fep3::Result jsonToResult(const Json::Value& value)
{
    return fep3::Result(value["error_code"].asInt(),
                        value["description"].asCString(),
                        value["line"].asInt(),
                        value["file"].asCString(),
                        value["function"].asCString());
}
} // namespace fep3::rpc::arya
///@endcond nodoc
