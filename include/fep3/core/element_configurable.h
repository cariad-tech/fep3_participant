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

#include "element_base.h"
#include <fep3/base/properties/propertynode.h>


namespace fep3
{
namespace core
{
namespace arya
{

using fep3::base::arya::PropertyVariable;

/**
 * This is the base class for every FEP3 Element which is able to register for properties
 * to implement your own FEP3 Element, derive from this class.
 */
class ElementConfigurable : public arya::ElementBase, public fep3::base::arya::Configuration {
public:
    /**
     * Deleted default CTOR
     */
    ElementConfigurable() = delete;
protected:
    /**
     * CTOR
     * this ctor will also create one property node called "element"
     *
     * @param[in] type_name Name of the type the element presents
     * @param[in] version_info Version information of the element
     */
    ElementConfigurable(std::string type_name, std::string version_info)
        : arya::ElementBase(type_name, version_info),
          fep3::base::arya::Configuration("element")
    {
    }
public:
    /**
     * Default DTOR
     *
     */
    virtual ~ElementConfigurable() = default;

private:
    /**
     * internal callback to load the element
     * this callback will create one property node called "element" within the configuration service
     * @remark the configuration service is required as component if using this class
     *
     * @param[in] components reference to the components. this pointer is valid until unload was called.
     *
     * @return Result error code
     * @retval ERR_NO_ERROR if succeeded
     * @retval ERR_NOT_FOUND arya::Configuration service required and not found
     */
    Result loadElement(const fep3::arya::IComponents& components) override
    {
        auto load_res = arya::ElementBase::loadElement(components);
        if (isFailed(load_res))
        {
            return load_res;
        }
        else
        {
            auto config_service = getComponents()->getComponent<fep3::arya::IConfigurationService>();
            if (config_service)
            {
                //we register ourself at the configuration service!
                return initConfiguration(*config_service);
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND, "configuration service required for the configurable elements");
            }
        }
    }
    /**
     * deinitializes the element
     *
     * @return Result error code
     * @retval ERR_NO_ERROR if succeeded
     */
    void unloadElement() override
    {
       deinitConfiguration();
       arya::ElementBase::unloadElement();
    }

};

} // namespace arya
using arya::ElementConfigurable;
using arya::PropertyVariable;
} // namespace core
} // namespace fep3

