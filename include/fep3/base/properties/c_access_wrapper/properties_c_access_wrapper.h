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
// @note All methods are defined inline to provide the functionality as header only.

#pragma once

#include <fep3/base/properties/c_intf/properties_c_intf.h>
#include <fep3/base/properties/properties_intf.h>
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
#include <fep3/plugin/c/destruction_manager.h>

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{
/// @cond no_documentation
namespace detail
{
    template<typename access_type, typename mutability_type>
    class Properties
    {};
    // class template specialization for fep3_arya_const_SIProperties
    template<typename access_type>
    class Properties<access_type, arya::Helper::Immutable>
        : public fep3::arya::IProperties
        , protected access::arya::Helper
    {
    public:
        inline Properties() = default;
        inline Properties(Properties&&) = delete;
        inline Properties(const Properties&) = delete;
        inline Properties& operator=(Properties&&) = delete;
        inline Properties& operator=(const Properties&) = delete;
        inline ~Properties() = default;
    private:
        // methods implementing non-const methods of fep3::arya::IProperties
        inline bool setProperty(const std::string&, const std::string&, const std::string&) override
        {
            throw Exception(fep3_plugin_c_interface_error_const_incorrectness);
        }
    public:
        // methods implementing const methods of fep3::arya::IProperties
        inline std::string getProperty(const std::string& name) const override;
        inline std::string getPropertyType(const std::string& name) const override;
        inline bool isEqual(const fep3::arya::IProperties& properties) const override;
        inline void copyTo(fep3::arya::IProperties& properties) const override;
        inline std::vector<std::string> getPropertyNames() const override;
    protected:
        /// Type of access structure
        access_type _access;
    };
    // class template specialization for fep3_arya_SIProperties
    template<typename access_type>
    class Properties<access_type, arya::Helper::Mutable>
        : public Properties<access_type, arya::Helper::Immutable> // decorate by const methods for immutable object
    {
    public:
        inline Properties() = default;
        inline Properties(Properties&&) = delete;
        inline Properties(const Properties&) = delete;
        inline Properties& operator=(Properties&&) = delete;
        inline Properties& operator=(const Properties&) = delete;
        inline ~Properties() = default;
        // methods implementing non-const methods of fep3::arya::IProperties
        inline bool setProperty(const std::string& name, const std::string& value, const std::string& type) override;
    };
} // namespace detail
/// @endcond no_documentation

/**
 * Class wrapping access to the C interface for @ref fep3::arya::IProperties.
 * Use this class to access a remote object of a type derived from @ref fep3::arya::IProperties that resides in another binary (e. g. a shared library).
 *
 * @tparam access_type The type of the C access structure for @ref fep3::arya::IProperties
 */
template<typename access_type>
class Properties
    : public detail::Properties<access_type, typename std::conditional
        <std::is_same<access_type, fep3_arya_SIProperties>::value
        , arya::Helper::Mutable
        , arya::Helper::Immutable
        >::type>
    , private c::arya::DestructionManager
{
public:
    /**
     * @brief CTOR
     * @param[in] access Access to the remote object
     * @param[in] destructors List of destructors to be called upon destruction of this
     */
    inline Properties
        (const access_type& access
        , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
        );
    /// Default CTOR
    inline Properties() = default;
    /// Deleted move CTOR
    inline Properties(Properties&&) = delete;
    /// Deleted copy CTOR
    inline Properties(const Properties&) = delete;
    /** Deleted move assignment
     * @return this Properties
     */
    inline Properties& operator=(Properties&&) = delete;
    /** Deleted copy assignment
     * @return this Properties
     */
    inline Properties& operator=(const Properties&) = delete;
    /// Default DTOR
    inline ~Properties() = default;
};

} // namespace arya
} // namespace access

namespace wrapper
{
namespace arya
{

/**
 * Wrapper class for interface \ref fep3::arya::IProperties
 *
 * @tparam handle_type The type of the handle to a remote object of @ref fep3::arya::IProperties
 */
template<typename handle_type>
class Properties : private arya::Helper<typename std::conditional
    <std::is_same<handle_type, fep3_arya_const_HIProperties>::value
    , const fep3::arya::IProperties
    , fep3::arya::IProperties
    >::type>
{
private:
    using WrapperHelper = arya::Helper<typename std::conditional
        <std::is_same<handle_type, fep3_arya_const_HIProperties>::value
        , const fep3::arya::IProperties
        , fep3::arya::IProperties
        >::type>;

public:
    /**
     * Calls \ref fep3::arya::IProperties::setProperty on the object identified by \p handle
     * @param[in] handle The handle to the properties object to call \ref fep3::arya::IProperties::setProperty on
     * @param[out] result Pointer to the result of the call of @ref fep3::arya::IProperties::setProperty on the property object
     * @param[in] name The name of the property to set
     * @param[in] value The value of the property to set
     * @param[in] type The type of the property to set
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::setProperty
     */
    template<typename = typename std::enable_if<std::is_same<handle_type, fep3_arya_HIProperties>::value, void>>
    static inline fep3_plugin_c_InterfaceError setProperty
        (handle_type handle
        , bool* result
        , const char* name
        , const char* value
        , const char* type
        ) noexcept
    {
        return WrapperHelper::callWithResultParameter
            (handle
            , &fep3::arya::IProperties::setProperty
            , [](bool result)
                {
                    return result;
                }
            , result
            , name
            , value
            , type
            );
    }
    /**
     * Calls \ref fep3::arya::IProperties::getProperty(...) on the object identified by \p handle
     * @param[in] handle The handle to the properties object to call \ref fep3::arya::IProperties::getProperty on
     * @param[in] callback Pointer to callback function to be called with the property value string as
     *                 returned by the call to \ref fep3::arya::IProperties::getProperty on the property object
     * @param[in] destination Pointer to the destination to be passed to the callback
     * @param[in] name The name of the property to get the value of
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::getProperty
     */
    static inline fep3_plugin_c_InterfaceError getProperty
        (handle_type handle
        , void(*callback)(void*, const char*)
        , void* destination
        , const char* name
        ) noexcept
    {
        return WrapperHelper::callWithResultCallback
            (handle
            , &fep3::arya::IProperties::getProperty
            , callback
            , destination
            , [](const std::string& name)
                {
                    return name.c_str();
                }
            , name
            );
    }
    /**
     * Calls \ref fep3::arya::IProperties::getPropertyType(...) on the object identified by \p handle
     * @param[in] handle The handle to the properties object to call \ref fep3::arya::IProperties::getPropertyType on
     * @param[in] callback Pointer to callback function to be called with the property value string as
     *                 returned by the call to \ref fep3::arya::IProperties::getPropertyType on the property object
     * @param[in] destination Pointer to the destination to be passed to the callback
     * @param[in] name The name of the property to get the type of
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::getPropertyType
     */
    static inline fep3_plugin_c_InterfaceError getPropertyType
        (handle_type handle
        , void(*callback)(void*, const char*)
        , void* destination
        , const char* name
        ) noexcept
    {
        return WrapperHelper::callWithResultCallback
            (handle
            , &fep3::arya::IProperties::getPropertyType
            , callback
            , destination
            , [](const std::string& type)
                {
                    return type.c_str();
                }
            , name
            );
    }
    /**
     * Calls \ref fep3::arya::IProperties::isEqual(...) on the object identified by \p handle
     * @param[in] handle The handle to the properties object to call \ref fep3::arya::IProperties::isEqual on
     * @param[out] result Pointer to the result of the call of @ref fep3::arya::IProperties::isEqual on the property object
     * @param[in] properties_access The access structure to the properties to be checked for equality
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::isEqual
     */
    static inline fep3_plugin_c_InterfaceError isEqual
        (handle_type handle
        , bool* result
        , fep3_arya_const_SIProperties properties_access
        ) noexcept
    {
        return WrapperHelper::callWithResultParameter
            (handle
            , &fep3::arya::IProperties::isEqual
            , [](bool result)
                {
                    return result;
                }
            , result
            , access::arya::Properties<fep3_arya_const_SIProperties>(properties_access, {})
            );
    }
    /**
     * Calls \ref fep3::arya::IProperties::getPropertyNames(...) on the object identified by \p handle
     * @param[in] handle The handle to the properties object to call \ref fep3::arya::IProperties::getPropertyNames on
     * @param[in] callback Pointer to callback function to be called with the property name as
     *                 returned by the call to \ref fep3::arya::IProperties::getPropertyNames on the property object
     * @param[in] destination Pointer to the destination to be passed to the callback
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::getPropertyNames
     */
    static inline fep3_plugin_c_InterfaceError getPropertyNames
        (handle_type handle, void(*callback)(void*, const char*), void* destination) noexcept
    {
        return WrapperHelper::callWithRecurringResultCallback
            (handle
            , &fep3::arya::IProperties::getPropertyNames
            , callback
            , destination
            , [](const std::string& name)
                {
                    return name.c_str();
                }
            );
    }
    /**
     * Calls \ref fep3::arya::IProperties::copyTo(...) on the object identified by \p handle
     * @param[in] handle The handle to the properties object to call \ref fep3::arya::IProperties::copyTo on
     * @param[in] properties_access The access structure to the properties to be checked for equality
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IProperties::copyTo
     */
    static inline fep3_plugin_c_InterfaceError copyTo
        (handle_type handle
        , fep3_arya_SIProperties properties_access
        ) noexcept
    {
        access::arya::Properties<fep3_arya_SIProperties> properties(properties_access, {});
        return WrapperHelper::call
            (handle
            , &fep3::arya::IProperties::copyTo
            , properties
            );
    }
};

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

/// @cond no_documentation
template<typename access_type>
Properties<access_type>::Properties
    (const access_type& access
    , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
    )
{
    this->_access = access;
    addDestructors(std::move(destructors));
}

template<typename access_type>
bool detail::Properties<access_type, arya::Helper::Mutable>::setProperty(const std::string& name, const std::string& value, const std::string& type)
{
    return this->callWithResultParameter
        (this->_access._handle
        , this->_access.setProperty
        , name.c_str()
        , value.c_str()
        , type.c_str()
        );
}

template<typename access_type>
std::string detail::Properties<access_type, arya::Helper::Immutable>::getProperty(const std::string& name) const
{
    return callWithResultCallback<std::string>
        (_access._handle
        , _access.getProperty
        , [](auto result)
            {
                return result;
            }
        , name.c_str()
        );
}

template<typename access_type>
std::string detail::Properties<access_type, arya::Helper::Immutable>::getPropertyType(const std::string& name) const
{
    return callWithResultCallback<std::string>
        (_access._handle
        , _access.getPropertyType
        , [](auto result)
            {
                return result;
            }
        , name.c_str()
        );
}

template<typename access_type>
bool detail::Properties<access_type, arya::Helper::Immutable>::isEqual(const fep3::arya::IProperties& properties) const
{
    return callWithResultParameter
        (_access._handle
        , _access.isEqual
        , fep3_arya_const_SIProperties
            {reinterpret_cast<fep3_arya_const_HIProperties>(&properties)
            , wrapper::arya::Properties<fep3_arya_const_HIProperties>::getProperty
            , wrapper::arya::Properties<fep3_arya_const_HIProperties>::getPropertyType
            , wrapper::arya::Properties<fep3_arya_const_HIProperties>::isEqual
            , wrapper::arya::Properties<fep3_arya_const_HIProperties>::copyTo
            , wrapper::arya::Properties<fep3_arya_const_HIProperties>::getPropertyNames
            }
        );
}

template<typename access_type>
void detail::Properties<access_type, Helper::Immutable>::copyTo(fep3::arya::IProperties& properties) const
{
    return call
        (_access._handle
        , _access.copyTo
        , fep3_arya_SIProperties
            {reinterpret_cast<fep3_arya_HIProperties>(&properties)
            , wrapper::arya::Properties<fep3_arya_HIProperties>::setProperty
            , wrapper::arya::Properties<fep3_arya_HIProperties>::getProperty
            , wrapper::arya::Properties<fep3_arya_HIProperties>::getPropertyType
            , wrapper::arya::Properties<fep3_arya_HIProperties>::isEqual
            , wrapper::arya::Properties<fep3_arya_HIProperties>::copyTo
            , wrapper::arya::Properties<fep3_arya_HIProperties>::getPropertyNames
            }
        );
}

template<typename access_type>
std::vector<std::string> detail::Properties<access_type, arya::Helper::Immutable>::getPropertyNames() const
{
    return callWithRecurringResultCallback<std::vector<std::string>, const char*>
        (_access._handle
        , _access.getPropertyNames
        , [](const char* name)
            {
                return std::string(name);
            }
        , &std::vector<std::string>::push_back
        );
}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
