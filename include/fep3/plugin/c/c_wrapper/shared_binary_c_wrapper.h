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

#include <memory>

#include "../shared_binary_intf.h"
#include "../c_intf/shared_binary_c_intf.h"

namespace fep3
{
namespace plugin
{
namespace c
{
namespace wrapper
{
namespace arya
{

/**
 * @brief Class wrapping a shared binary to make it accessible via free functions
 */
class SharedBinary
{
public:
    /**
     * Destroys the object identified by \p handle
     * @param[in] handle The handle identifying the object to be destroyed
     */
    static inline void destroy(fep3_plugin_c_arya_HISharedBinary handle)
    {
        delete reinterpret_cast<std::shared_ptr<c::arya::ISharedBinary>*>(handle);
    }
};

} // namespace arya
using arya::SharedBinary;
} // namespace wrapper
} // namespace c
} // namespace plugin
} // namespace fep3
