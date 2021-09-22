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

#include <string>
#include <memory>
#include <list>
#include <map>
#include <fep3/fep3_participant_export.h>
#include <fep3/components/base/components_intf.h>


namespace fep3
{
namespace arya
{

/**
 * This is the interface of one element the participant can deal with class for every FEP3 Element.
 * To implement your own FEP3 Element, derive from this class.
 */
class IElement
{

public:
    /// DTOR
    virtual ~IElement() = default;

public:
    /**
     * Returns the typename of the element.
     * The typename represents the type the element is implementing.
     * @remark This is not the instance name of the element!
     *         The instance name is usually the same as the name of the participant this element is loaded in.
     *
     * @return std::string The typename
     */
    virtual std::string getTypename() = 0;
    /**
     * Returns the version of the element.
     * The version of the element implementation.
     * @remark This is the instance version of the element type
     *         and will be used only for information at the moment!
     *         There is no further functionality or checks on that!
     *
     * @return std::string The version as string (this is vendor dedendent, and only for information!)
     */
    virtual std::string getVersion() = 0;
    /**
     * Loads internals of the element
     *
     * @param[in] components Reference to the components. This pointer is valid until @ref unloadElement was called.
     *
     * @return Result error code
     * @retval NO_ERROR if succeded
     */
    virtual Result loadElement(const arya::IComponents& components) = 0;
    /**
     * Unloads internals of the element
     *
     * @return Result error code
     * @retval NO_ERROR if succeded
     */
    virtual void unloadElement() = 0;
    /**
     * Initializes the element
     *
     * @return Result error code
     * @retval NO_ERROR if succeded
     */
    virtual Result initialize() = 0;
    /**
     * Deinitializes the element
     *
     */
    virtual void deinitialize() = 0;

    /**
     * Runs the element
     *
     * @return Result error code
     * @retval NO_ERROR if succeded
     */
    virtual Result run() = 0;
    /**
     * Stops the element
     */
    virtual void stop() = 0;
};

} // namespace arya
using arya::IElement;
} // namespace fep3
