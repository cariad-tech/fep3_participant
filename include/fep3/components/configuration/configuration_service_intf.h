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

#include <fep3/fep3_participant_types.h>
#include <fep3/fep3_errors.h>
#include <fep3/components/base/component_iid.h>

#include "propertynode_intf.h"

namespace fep3
{
namespace arya
{

/**
* @brief The configuration service is a tree based container for configuration information.
*
* It's purpose is to act as a central instance for providing configuration information of a participant.
* Components can use the service to access configuration information of other components and provide their one.
* The service manages property nodes implementing @ref fep3::arya::IPropertyNode.
* An @ref fep3::arya::IPropertyNode stores a value as well as a list of childs of type @ref fep3::arya::IPropertyNode.
* Typically the content of the @ref fep3::arya::IConfigurationService is also available via rpc.
*/
class IConfigurationService
{

public:
    /// the fep component interface identifier for the IConfigurationService
    FEP_COMPONENT_IID("configuration_service.arya.fep3.iid");

protected:
    /// DTOR
    ~IConfigurationService() = default;

public:
    /**
     * @brief Register the node @p property_node as main node.
     * The node name of @p property_node has to be unique within all registered main nodes.
     *
     * @param[in] property_node Node to register at the configuration service.
     * @return fep3::Result
     * @retval ERR_RESOURCE_IN_USE if a node with the same name is already registered.
     */
    virtual fep3::Result registerNode(std::shared_ptr<fep3::arya::IPropertyNode> property_node) = 0;

    /**
     * @brief Unregister a main node with @p name
     *
     * @param[in] name Name of the node to unregister
     * @return fep3::Result
     * @retval ERR_NOT_FOUND if a node with @p name is not registered.
     */
    virtual fep3::Result unregisterNode(const std::string& name) = 0;

    /**
     * @brief Checks whether the node with @p path is registered with the service.
     *
     * @param[in] path Path of the node to search for.
     * @return @c true if the node is registered with the service, @c false otherwise.
     */
    virtual bool isNodeRegistered(const std::string& path) const = 0;

    /**
     * @brief Searches for a node with @p path.
     *          The root node can only be returned using @ref getConstNode
     *          The node can be used to either read or change a property.
     * @param[in] path Path of the node to search for and return (e.g. Clock, Clock/MainClock etc.)
     * @return shared_ptr to the node
     * @retval Default constructed shared_ptr if node path is not valid
     * @retval Default constructed shared_ptr if node was not found.
     */
    virtual std::shared_ptr<fep3::arya::IPropertyNode> getNode(const std::string& path) const = 0;

    /**
     * @brief Searches for a node with @p path.
     *
     * @param[in] path Path of the node to search for and return (e.g. Clock, Clock/MainClock etc.).
     *              If @p path is provided as an empty string, the root node will be returned.
     * @return shared_ptr to the node
     * @retval Default constructed shared_ptr if node path is not valid.
     * @retval Default constructed shared_ptr if node was not found.
     */
    virtual std::shared_ptr<const fep3::arya::IPropertyNode> getConstNode(const std::string& path = "") const = 0;

};

} // namespace arya
using arya::IConfigurationService;
} // namespace fep3