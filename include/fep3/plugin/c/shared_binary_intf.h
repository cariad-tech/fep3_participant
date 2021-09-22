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

namespace fep3
{
namespace plugin
{
namespace c
{
namespace arya
{

/**
 * @brief Interface for shared binary
 * A shared binary implements shared ownership to a binary, e. g. a dynamically loaded shared library (aka plugin).
 * As long as the shared binary exists, the underlying resource, e. g. the library, must not be unloaded.
 * To ensure that a resource (e. g. a library) is not unloaded while any object exists that resides in the resource
 * (i. e. was created in the memory space of the resource), just retain a copy of the shared binary alongside with every such object.
 *
 */
class ISharedBinary
{
public:
    /// DTOR
    virtual ~ISharedBinary() = default;
};

} // namespace arya
using arya::ISharedBinary;
} // namespace c
} // namespace plugin
} // namespace fep3
