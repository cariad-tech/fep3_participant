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

#include <fep3/core/default_job_element.h>
#include <fep3/participant/element_factory_intf.h>

namespace fep3 {
namespace core {
namespace detail {

/// @cond nodoc
template <typename ElementType>
struct ICustomElementCreator {
    virtual ~ICustomElementCreator() = default;
    virtual std::unique_ptr<ElementType> createElement() = 0;
};

template <typename ElementType, typename... Args>
struct CustomElementCreator : ICustomElementCreator<ElementType> {
    CustomElementCreator(Args&&... args) : _args(std::forward<Args>(args)...)
    {
    }

    CustomElementCreator(const CustomElementCreator&) = delete;
    CustomElementCreator(CustomElementCreator&&) = delete;
    ~CustomElementCreator() = default;

    std::unique_ptr<ElementType> createElement() override
    {
        return std::apply(
            [&](auto&&... args) { return std::make_unique<ElementType>(std::move(args)...); },
            _args);
    }

    std::tuple<Args...> _args;
};
/// @endcond nodoc

/**
 * Custom element factory
 * @tparam CustomElement the element type to create. Derive from fep3::core::Element and go
 * further with that.
 */
template <typename CustomElement>
class CustomElementFactory : public fep3::base::IElementFactory {
public:
    /// @cond nodoc
    template <typename... Args>
    CustomElementFactory(Args&&... args)
        : _custom_element_creator(std::make_unique<CustomElementCreator<CustomElement, Args...>>(
              std::forward<Args>(args)...))
    {
    }
    /// @endcond nodoc

    /**
     * @brief Deleted Copy CTOR
     */
    CustomElementFactory(const CustomElementFactory&) = delete;

    /**
     * @brief Deleted Move CTOR
     */
    CustomElementFactory(CustomElementFactory&&) = delete;

    /**
     * @brief Deleted Copy assignment operator
     *
     * @return ElementFactory&
     */
    CustomElementFactory& operator=(const CustomElementFactory&) = delete;

    /**
     * @brief Deleted Move assignment operator
     *
     * @return ElementFactory&
     */
    CustomElementFactory& operator=(CustomElementFactory&&) = delete;

    /**
     * @brief Default DTOR
     */
    ~CustomElementFactory() = default;

    /**
     * Creates the element
     *
     * @returns unique pointer the created element.
     */
    std::unique_ptr<fep3::base::IElement> createElement(const fep3::IComponents&) const override
    {
        return std::make_unique<fep3::core::DefaultJobElement<CustomElement>>(
            (_custom_element_creator->createElement()));
    }

private:
    ///@cond nodoc
    std::unique_ptr<ICustomElementCreator<CustomElement>> _custom_element_creator;
    ///@endcond nodoc
};

} // namespace detail
using detail::CustomElementFactory;
} // namespace core
} // namespace fep3
