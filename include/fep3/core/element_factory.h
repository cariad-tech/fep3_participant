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
#include <fep3/participant/element_factory_intf.h>

namespace fep3
{
namespace core
{
namespace arya
{

/**
 * Simple element factory
 * @tparam element_type the element_type to create. Derive from fep3::core::ElementBase and go further with that.
 */
template <typename element_type>
class ElementFactory : public fep3::arya::IElementFactory
{
public:
    /**
     * CTOR for the Element factory which is able to create elements with a specified
     *
     * @returns Shared pointer to the created element.
     */
    ElementFactory()
    {
    }
    /**
     * Creates the element
     *
     * @returns unique pointer to the created element.
     */
    std::unique_ptr<fep3::arya::IElement> createElement(const fep3::arya::IComponents& /*components*/) const override
    {
        return std::unique_ptr<fep3::arya::IElement>(new element_type());
    }
};

} // namespace arya
using arya::ElementFactory;
} // namespace core
} // namespace fep3
