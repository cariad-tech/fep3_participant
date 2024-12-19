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

#include <fep3/fep3_errors.h>

#include <memory>

namespace fep3 {
namespace arya {

/**
 * @brief Get the Component Interface ID for the given interface type T
 * The interface type T must define the interface by the helper macro (@ref FEP_COMPONENT_IID)
 *
 * @tparam T The interface type
 * @return std::string
 */
template <class T>
std::string getComponentIID()
{
    return T::getComponentIID();
}

// forward decl
class IComponents;

/**
 * @brief Base interface of a component as part of a @ref IComponent registry
 */
class IComponent {
public:
    /// DTOR
    virtual ~IComponent() = default;

    /**
     * @brief Creates internals of the component
     *
     * @param[in] components Weak pointer to the components
     * @return fep3::Result
     */
    virtual fep3::Result createComponent(
        const std::weak_ptr<const arya::IComponents>& components) = 0;

    /**
     * @brief Destroys internals of the component.
     * @note This does not destroy the component.
     *
     * @return fep3::Result
     */
    virtual fep3::Result destroyComponent() = 0;

    /**
     * @brief Initializes a component
     *
     * @return fep3::Result
     */
    virtual fep3::Result initialize() = 0;

    /**
     * @brief Gets the component ready for running state
     *
     * @return fep3::Result
     */
    virtual fep3::Result tense() = 0;

    /**
     * @brief Makes the component falling back to a simple initialized state
     * Relax is the antonym of @ref tense.
     *
     * @return fep3::Result
     */
    virtual fep3::Result relax() = 0;

    /**
     * @brief Starts the component
     *
     * @return fep3::Result
     */
    virtual fep3::Result start() = 0;

    /**
     * @brief Stops the component
     *
     * @return fep3::Result
     */
    virtual fep3::Result stop() = 0;

    /**
     * @brief Pauses the component
     *
     * @return fep3::Result
     */
    virtual fep3::Result pause() = 0;

    /**
     * @brief Deinitializes the component
     *
     * @return fep3::Result
     */
    virtual fep3::Result deinitialize() = 0;

    /**
     * @brief Get the interface requested by the @p iid and return a void pointer to it
     *
     * @param[in] iid
     * @return Pointer to the interface of the component if this component has the requested @p idd,
     *         nullptr otherwise
     */
    virtual void* getInterface(const std::string& iid) = 0;
};

} // namespace arya

using arya::IComponent;

/**
 * @brief extracting @ref fep3::arya::getComponentIID() from version namespace
 * @tparam interface_type the component interface type
 * @return the component interface id of the given @p interface_type
 */
using arya::getComponentIID;

} // namespace fep3
