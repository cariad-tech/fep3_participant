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

inline Json::Value resultToJson(fep3::Result nResult)
{
    Json::Value oResult;
    oResult["error_code"] = nResult.getErrorCode();
    oResult["description"] = nResult.getDescription();
    oResult["line"] = nResult.getLine();
    oResult["file"] = nResult.getFile();
    oResult["function"] = nResult.getFunction();

    return oResult;
}
} // namespace fep3::rpc::arya
///@endcond nodoc
