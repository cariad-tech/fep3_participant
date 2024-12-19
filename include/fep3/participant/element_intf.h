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

#include <fep3/components/base/components_intf.h>

namespace fep3 {
namespace base {

/**
 * This is the interface of one element the participant can deal with class for every FEP3 Element.
 * To implement your own FEP3 Element, derive from this class.
 */
class IElement {
public:
    /// DTOR
    virtual ~IElement() = default;

public:
    /**
     * Returns the type name of the element.
     * The type name represents the type the element is implementing.
     * @remark This is not the instance name of the element!
     *         The instance name is usually the same as the name of the participant this element is
     *         loaded in.
     *
     * @return std::string Result of getTypename
     */
    virtual std::string getTypename() const = 0;

    /**
     * Returns the version of the element.
     * The version of the element implementation.
     * @remark This is the instance version of the element type
     *         and will be used only for information at the moment!
     *         There is no further functionality or checks on that!
     *
     * @return std::string The version as string (this is vendor dependent, and only for
     * information!)
     */
    virtual std::string getVersion() const = 0;

    /**
     * Loads internals of the element
     *
     * @param[in] components Reference to the components. This pointer is valid until @ref
     * unloadElement was called.
     *
     * @return Result error code
     * @retval NO_ERROR if succeeded
     */
    virtual Result loadElement(const fep3::IComponents& components) = 0;

    /**
     * Unloads internals of the element
     */
    virtual void unloadElement() = 0;

    /**
     * Initializes the element
     *
     * @return Result error code
     * @retval NO_ERROR if succeeded
     */
    virtual Result initialize() = 0;

    /**
     * Deinitializes the element
     */
    virtual void deinitialize() = 0;

    /**
     * Runs the element
     *
     * @return Result error code
     * @retval NO_ERROR if succeeded
     */
    virtual Result run() = 0;

    /**
     * Stops the element
     */
    virtual void stop() = 0;
};

} // namespace base

/// @cond nodoc
namespace arya {
using IElement [[deprecated(
    "Since 3.1, fep3::arya::IElement is deprecated. Please use fep3::base::IElement")]] =
    fep3::base::IElement;
} // namespace arya

using IElement
    [[deprecated("Since 3.1, fep3::IElement is deprecated. Please use fep3::base::IElement")]] =
        fep3::base::IElement;
/// @endcond nodoc
} // namespace fep3
