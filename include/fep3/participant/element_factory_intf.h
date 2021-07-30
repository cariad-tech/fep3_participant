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
#include <fep3/fep3_participant_types.h>
#include "element_intf.h"
#include <fep3/components/base/components_intf.h>

namespace fep3
{
namespace arya
{

/**
 * Interface for the element factory
 *
 */
class IElementFactory
{
protected:
    /// DTOR
    ~IElementFactory() = default;

public:
    /**
     * Creates the element
     *
     * @param[in] components components reference to provide the component access
     * @returns Shared pointer to the created element.
     */
    virtual std::unique_ptr<arya::IElement> createElement(const arya::IComponents& components) const = 0;
};

} // namespace arya
using arya::IElementFactory;
} // namespace fep3
