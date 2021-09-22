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

#include <string>

#include <fep3/fep3_result_decl.h>
#include <fep3/fep3_optional.h>

namespace fep3
{
// Note: this namespace provides functionality similar to a subset of a_util version 4.x.x "cSystem" class.
// As such funcionality is not yet available in a_util 5.x.x, it was added here
namespace environment_variable
{

Optional<std::string> get(const std::string& strName);
Result set(const std::string& name, const std::string& value);

} // namespace environment_variable
} // namespace fep3