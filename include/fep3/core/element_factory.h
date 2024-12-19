/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/participant/element_factory_intf.h>

namespace fep3 {
namespace core {

/**
 * Simple element factory
 * @tparam element_type the element_type to create. Derive from fep3::core::ElementBase and go
 * further with that.
 */
template <typename element_type>
class ElementFactory : public fep3::base::IElementFactory {
public:
    /**
     * CTOR for the Element factory which is able to create elements with a specified type.
     */
    ElementFactory() = default;

    /**
     * @brief Deleted Copy CTOR
     */
    ElementFactory(const ElementFactory&) = delete;

    /**
     * @brief Deleted Move CTOR
     */
    ElementFactory(ElementFactory&&) = delete;

    /**
     * @brief Deleted Copy assignment operator
     *
     * @return ElementFactory&
     */
    ElementFactory& operator=(const ElementFactory&) = delete;

    /**
     * @brief Deleted Move assignment operator
     *
     * @return ElementFactory&
     */
    ElementFactory& operator=(ElementFactory&&) = delete;

    /**
     * @brief Default DTOR
     */
    virtual ~ElementFactory() = default;

    /**
     * Creates the element
     *
     * @returns unique pointer to the created element.
     */
    std::unique_ptr<fep3::base::IElement> createElement(
        const fep3::IComponents& /*components*/) const override
    {
        return std::unique_ptr<fep3::base::IElement>(new element_type());
    }
};

/// @cond nodoc
namespace arya {
template <typename element_type>
using ElementFactory [[deprecated("Since 3.1, fep3::arya::ElementFactory is deprecated. Please use "
                                  "fep3::core::ElementFactory")]] =
    fep3::core::ElementFactory<element_type>;
} // namespace arya
/// @endcond nodoc

} // namespace core
} // namespace fep3
