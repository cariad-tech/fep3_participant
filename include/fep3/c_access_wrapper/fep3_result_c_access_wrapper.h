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

#include <fep3/fep3_result_decl.h>
#include <fep3/c_intf/fep3_result_c_intf.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{

/**
 * @brief Helper function for result conversion from @ref fep3_SResult to fep3::Result
 * @param[in] result The result to be converted to fep3::Result
 * @return The converted result
 */
inline fep3::Result getResult(const fep3_SResult& result)
{
    // optimization: prevent heap allocation (for the description in fep3::Result) if error code is 0
    // by returning an empty Result
    if(0 == result._error_code)
    {
        return fep3::Result{};
    }
    else
    {
        return fep3::Result
            (result._error_code
            , result._error_description
            , result._line
            , result._file
            , result._function
            );
    }
}

} // namespace arya
} // namespace access

namespace wrapper
{
namespace arya
{

/**
 * @brief Helper function for result conversion from fep3::Result to @ref fep3_SResult
 *
 * @param[in] result The result to be converted to @ref fep3_SResult
 * @return The converted result
 */
inline fep3_SResult getResult(const fep3::Result& result)
{
    return fep3_SResult
        {result.getErrorCode()
        , result.getLine()
        , result.getDescription()
        , result.getFile()
        , result.getFunction()
        };
}

} // namespace arya
} // namespace wrapper
} // namespace c
} // namespace plugin
} // namespace fep3