/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/participant/element_factory_intf.h>

namespace fep3 {
namespace arya {

/**
 * @brief Class managing an element
 * This class loads/unloads an element, controls its lifetime and provides access to Element
 * operations. It also manages data jobs related to the element
 */
class ElementManager {
public:
    /**
     * Default CTOR
     */
    ElementManager() = default;
    ElementManager(const std::shared_ptr<const base::IElementFactory>& element_factory);

    ElementManager(const ElementManager&) = delete;
    ElementManager& operator=(const ElementManager&) = delete;

    ElementManager(ElementManager&&) = default;
    ElementManager& operator=(ElementManager&&) = default;

    /**
     * DTOR Destroys the component registry
     */
    virtual ~ElementManager();

    /**
     * Loads the element.
     *
     * @return Result 'NO_ERROR' if succeeded, error code otherwise
     */
    virtual Result loadElement(const IComponents& components);

    /**
     * Unloads the element (if loaded)
     */
    virtual void unloadElement();

    /**
     * Initializes the element.
     *
     * @return Result 'NO_ERROR' if succeeded, error code otherwise
     */
    virtual Result initializeElement();

    /**
     * Deinitializes the element.
     */
    virtual void deinitializeElement();

    /**
     * Runs the element
     *
     * @return Result 'NO_ERROR' if succeeded, error code otherwise
     */
    Result runElement();

    /**
     * Stops the element.
     */
    virtual void stopElement();

private:
    std::unique_ptr<fep3::base::IElement> _element;
    std::shared_ptr<const base::IElementFactory> _element_factory;
};

} // namespace arya
using arya::ElementManager;
} // namespace fep3
