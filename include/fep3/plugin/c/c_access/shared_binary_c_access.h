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
#include <deque>

#include <fep3/plugin/c/shared_binary_intf.h>
#include <fep3/plugin/c/c_intf/shared_binary_c_intf.h>

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
 * @brief Class providing access to a (remote) shared binary
 */
class SharedBinary : public c::arya::ISharedBinary
{
public:
    /**
     * CTOR
     * @param[in] shared_binary_access C access structure to the remote shared binary object
     */
    SharedBinary(const fep3_plugin_c_arya_SISharedBinary& shared_binary_access)
        : _shared_binary_access_list({shared_binary_access})
    {}
    /**
     * CTOR
     * @param[in] shared_binary_access_list List of C access structures to the remote shared binary objects
     */
    SharedBinary(const std::initializer_list<fep3_plugin_c_arya_SISharedBinary>& shared_binary_access_list)
        : _shared_binary_access_list(shared_binary_access_list)
    {}
    /**
     * DTOR calling destroy on all C access structures
     */
    virtual ~SharedBinary() override
    {
        for(const auto& shared_binary_access : _shared_binary_access_list)
        {
            shared_binary_access.destroy(shared_binary_access._handle);
        }
    }
private:
    std::deque<fep3_plugin_c_arya_SISharedBinary> _shared_binary_access_list;
};

} // namespace arya
using arya::SharedBinary;
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
