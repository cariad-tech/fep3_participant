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
#ifndef _FEP3_COMP_PROPERTIES_INTF_H_
#define _FEP3_COMP_PROPERTIES_INTF_H_

#include <string>
#include <vector>
#include <memory>

namespace fep3
{
namespace arya
{

/**
 * @brief Properties interfaces to access a list of key value pairs
 *
 */
class IProperties
{
public:
    /// DTOR
    virtual ~IProperties() = default;

    /**
     * @brief sets the value and type of the given property.
     * If the property not exists it will add one.
     * If the type is different, than the existing one it will change it.
     *
     * @param[in] name  name of the property (this is not a path, a single name)
     * @param[in] value the value as string
     * @param[in] type the string description of the type
     *             There are more types possible than the default types: fep3::base::PropertyType
     * @return @c true if the value could be set @c false otherwise (There might be an exception
     * depending on the implementation of the interface)
     */
    virtual bool setProperty(const std::string& name,
        const std::string& value,
        const std::string& type) = 0;
    /**
     * @brief gets the property value as string
     *
     * @param[in] name Name of the property
     * @return std::string the value as string
     *                     you may determine the type by using @ref getPropertyType.
     */
    virtual std::string getProperty(const std::string& name) const = 0;

    /**
     * @brief gets the property value
     *
     * @param[in] name name of the property
     * @return std::string the type of the property.
     *                     default types are define by fep3::base::PropertyType
     */
    virtual std::string getPropertyType(const std::string& name) const = 0;

    /**
     * @brief compares this key value list with the given properties instance
     * the properties are equal if each property of this will have the same value within \p properties
     *
     * @param[in] properties the properties instance to compare to
     * @return @c true if each properties of this have the same value within \p properties,
     * @c false otherwise
     */
    virtual bool isEqual(const IProperties& properties) const = 0;
    /**
     * @brief assignment helper
     *
     * @param[in] properties properties to copy values of this property object to
     */
    virtual void copyTo(IProperties& properties) const = 0;

    /**
     * @brief returns a list of all property names of this node
     *
     * @return vector of property names
     */
    virtual std::vector<std::string> getPropertyNames() const = 0;
};


}  // namespace arya
using arya::IProperties;
} // namespace fep3

#endif //_FEP3_COMP_PROPERTIES_INTF_H_