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

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

/**
 * @brief Interface for the component getter function getter
 */
class IComponentGetterFunctionGetter
{
protected:
    /// DTOR
    ~IComponentGetterFunctionGetter() = default;

public:
    /**
     * Parenthesis operator to get the component getter function
     * @param[in] iid Interface ID of the component to get the getter function of
     * @return void pointer to the component getter function
     */
    virtual void* operator()(const std::string& iid) const = 0;
};

} // namespace arya
} // namespace c
} // namespace plugin
} // namespace fep3