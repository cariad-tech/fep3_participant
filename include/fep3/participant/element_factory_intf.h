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

#include <fep3/participant/element_intf.h>

namespace fep3 {
namespace base {

/**
 * Interface for the element factory
 */
class IElementFactory {
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
    virtual std::unique_ptr<base::IElement> createElement(
        const fep3::IComponents& components) const = 0;
};
} // namespace base

/// @cond nodoc
namespace arya {
using IElementFactory [[deprecated("Since 3.1, fep3::arya::IElementFactory is deprecated. Please "
                                   "use fep3::base::IElementFactory")]] =
    fep3::base::IElementFactory;
} // namespace arya
/// @endcond nodoc

} // namespace fep3
