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

//Guideline - FEP System Library API Exception
#ifndef _FEP3_BASE_PROPERTIES_H_
#define _FEP3_BASE_PROPERTIES_H_

#include "properties_intf.h"

#include <map>
#include <string>
#include <tuple>
#include <vector>
#include <algorithm>
#include <iterator>

namespace fep3 {
namespace base {
namespace arya {

/**
 * @brief Implementation class to represent a typed key value list
 *
 * @tparam T the property interface
 */
template <class T = fep3::arya::IProperties>
class Properties : public T {
public:
    /**
     * @brief sets the value and type of the given property.
     * If the property not exists it will add one.
     * If the type is different, than the existing one it will change it.
     *
     * @param[in] name  name of the property (this is not a path, a single name)
     * @param[in] value the value as string
     * @param[in] type the string description of the type
     *             There are more types possible than the default types: @ref fep3::base::arya::PropertyType
     * @return @c true if the value could be set, @c false otherwise
     */
    bool setProperty(const std::string& name,
        const std::string& value,
        const std::string& type) override
    {
        std::tuple<std::string, std::string> tuple_to_add(value, type);
        _properties[name] = tuple_to_add;
        return true;
    }
    /**
     * @brief gets the property value as string
     *
     * @param[in] name Name of the property
     * @return std::string the value as string
     *                     you may determine the type by using @ref getPropertyType
     */
    std::string getProperty(const std::string& name) const override
    {
        auto val = _properties.find(name);
        if (val != _properties.end())
        {
            return std::get<0>(val->second);
        }
        else
        {
            return std::string();
        }
    }

    /**
     * @brief gets the property value
     *
     * @param[in] name name of the property
     * @return std::string the type of the property.
     *                     default types are define by @ref fep3::base::arya::PropertyType
     */
    std::string getPropertyType(const std::string& name) const override
    {
        auto val = _properties.find(name);
        if (val != _properties.end())
        {
            return std::get<1>(val->second);
        }
        else
        {
            return std::string();
        }
    }
    /**
     * @brief compares this key value list with the given properties instance
     * the properties are equal if each property of this will have the same value within \p properties
     *
     * @param[in] properties the properties instance to compare to
     * @return @c true if each properties of this have the same value within \p properties,
               @c false otherweise
     */
    bool isEqual(const fep3::arya::IProperties& properties) const override
    {
        for (const auto& current : _properties)
        {
            if (std::get<0>(current.second) != properties.getProperty(current.first))
            {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief assignment helper
     *
     * @param[in,out] properties properties to copy values of this property object to
     */
    void copyTo(fep3::arya::IProperties& properties) const override
    {
        for (const auto& current : _properties)
        {
            properties.setProperty(current.first,
                std::get<0>(current.second),
                std::get<1>(current.second));
        }
    }

    /**
     * @brief returns a list of all property names of this node
     *
     * @return vector of property names
     */
    std::vector<std::string> getPropertyNames() const override
    {
        std::vector<std::string> retval;
        retval.reserve(_properties.size());
        for (const auto& current : _properties)
        {
            retval.push_back(current.first);
        }
        return retval;
    }

    /**
     * @brief returns a list of all property values of this node
     *
     * @return vector of property values
     */
    std::vector<std::string> getPropertyValues() const
    {
        std::vector<std::string> retval;
        retval.reserve(_properties.size());
        for (const auto& current : _properties)
        {
            retval.push_back(std::get<0>(current.second));
        }
        return retval;
    }

    /**
     * @brief returns a list of all property types of this node
     *
     *  @return vector of property types
     */
    std::vector<std::string> getPropertyTypes() const
    {
        std::vector<std::string> retval;
        retval.reserve(_properties.size());
        for (const auto& current : _properties)
        {
            retval.push_back(std::get<1>(current.second));
        }
        return retval;
    }

protected:
    ///key value map
    std::map<std::string, std::tuple<std::string, std::string>> _properties;
};
} // namespace arya

using arya::Properties;

} // namespace base
} // namespace fep3

#endif //_FEP3_BASE_PROPERTIES_INTF_H_