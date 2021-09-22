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

#include <cstring> // for strcmp

#include <fep3/fep3_macros.h>
#include <fep3/components/base/c_access_wrapper/component_base_c_access.h>
#include <fep3/components/configuration/configuration_service_intf.h>
#include <fep3/components/configuration/c_intf/configuration_service_c_intf.h>
#include <fep3/components/configuration/c_access_wrapper/property_node_c_access_wrapper.h>
#include <fep3/components/base/c_access_wrapper/component_c_wrapper.h>

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

/**
 * @brief Access class for @ref fep3::arya::IConfigurationService.
 * Use this class to access a remote object of a type derived from IConfigurationService that resides in another binary (e. g. a shared library).
 */
class ConfigurationService : public ::fep3::plugin::c::access::arya::ComponentBase<::fep3::arya::IConfigurationService>
{
public:
    /// Symbol name of the create function that is capable to create a Configuration Service
    static constexpr const char* const create_function_name = FEP3_EXPAND_TO_STRING(SYMBOL_fep3_plugin_c_arya_createConfigurationService);
    /// Gets the function to get an instance of a Configuration Service that resides in a C plugin
    static decltype(&fep3_plugin_c_arya_getConfigurationService) getGetterFunction()
    {
        return fep3_plugin_c_arya_getConfigurationService;
    }
    /// Type of access structure
    using Access = fep3_arya_SIConfigurationService;

    /**
     * CTOR
     *
     * @param[in] access Access to the remote object
     * @param[in] shared_binary Shared pointer to the binary this resides in
     */
    inline ConfigurationService(Access&& access, const std::shared_ptr<c::arya::ISharedBinary>& shared_binary);
    /// Deleted move CTOR
    inline ConfigurationService(ConfigurationService&&) = delete;
    /// Deleted copy CTOR
    inline ConfigurationService(const ConfigurationService&) = delete;
    /** Deleted move assignment
     * @return this ConfigurationService
     */
    inline ConfigurationService& operator=(ConfigurationService&&) = delete;
    /** Deleted copy assignment
     * @return this ConfigurationService
     */
    inline ConfigurationService& operator=(const ConfigurationService&) = delete;
    /**
     * DTOR destroying the corresponding remote object
     */
    inline ~ConfigurationService() override = default;

    /// @cond no_documentation
    // methods implementing fep3::arya::IConfigurationService
    inline fep3::Result registerNode(std::shared_ptr<fep3::arya::IPropertyNode> property_node) override;
    inline fep3::Result unregisterNode(const std::string& name) override;
    inline bool isNodeRegistered(const std::string& path) const override;
    inline std::shared_ptr<fep3::arya::IPropertyNode> getNode(const std::string& path) const override;
    inline std::shared_ptr<const fep3::arya::IPropertyNode> getConstNode(const std::string& path) const override;
    /// @endcond no_documentation

private:
    Access _access;
};

} // namespace arya
} // namespace access

namespace wrapper
{
namespace arya
{

/**
 * Wrapper class for interface @ref fep3::arya::IConfigurationService
 */
class ConfigurationService : private arya::Helper<fep3::arya::IConfigurationService>
{
private:
    using Helper = arya::Helper<fep3::arya::IConfigurationService>;
    using Handle = fep3_arya_HIConfigurationService;

public:
    /// @cond no_documentation
    static inline fep3_plugin_c_InterfaceError registerNode
        (fep3_arya_HIConfigurationService handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        , fep3_plugin_c_arya_SDestructionManager destruction_manager_access
        , fep3_arya_SIPropertyNode property_node_access
        ) noexcept
    {
        return Helper::transferSharedPtrWithResultCallback<access::arya::PropertyNode<fep3_arya_SIPropertyNode>>
            (handle
            , [](auto&& configuration_service, auto&& property_node)
                {
                    return configuration_service->registerNode(property_node);
                }
            , result_callback
            , result_destination
            , &getResult
            , destruction_manager_access
            , property_node_access
            );
    }

    static inline fep3_plugin_c_InterfaceError unregisterNode
        (Handle handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        , const char* name
        ) noexcept
    {
        return Helper::callWithResultCallback
            (handle
            , &fep3::arya::IConfigurationService::unregisterNode
            , result_callback
            , result_destination
            , &getResult
            , name
            );
    }

    static inline fep3_plugin_c_InterfaceError isNodeRegistered
        (Handle handle
        , bool* result
        , const char* path
        ) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IConfigurationService::isNodeRegistered
            , [](bool result)
                {
                    return result;
                }
            , result
            , path
            );
    }

    static inline fep3_plugin_c_InterfaceError getNode
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access
        , fep3_arya_SIPropertyNode* property_node_access
        , const char* path
        ) noexcept
    {
        return Helper::getSharedPtr
            (handle
            , &fep3::arya::IConfigurationService::getNode
            , destruction_manager_access
            , property_node_access
            , wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::AccessCreator()
            , path
            );
    }

    static inline fep3_plugin_c_InterfaceError getConstNode
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access
        , fep3_arya_const_SIPropertyNode* property_node_access
        , const char* path
        ) noexcept
    {
        return Helper::getSharedPtr
            (handle
            , &fep3::arya::IConfigurationService::getConstNode
            , destruction_manager_access
            , property_node_access
            , wrapper::arya::PropertyNode<fep3_arya_const_SIPropertyNode>::AccessCreator()
            , path
            );
    }
    /// @endcond no_documentation

private:
    /// Type of access structure
    using Access = fep3_arya_SIConfigurationService;
};

namespace detail
{
/// @cond no_documentation
inline fep3_plugin_c_InterfaceError getConfigurationService
    (fep3_arya_SIConfigurationService* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    if (0 == strcmp(::fep3::arya::IConfigurationService::getComponentIID(), iid))
    {
        return ::fep3::plugin::c::wrapper::arya::get<::fep3::arya::IComponent, ::fep3::arya::IConfigurationService>
            (access_result
            , handle_to_component
            , [](::fep3::arya::IConfigurationService* pointer_to_object)
        {
            return fep3_arya_SIConfigurationService
                { reinterpret_cast<fep3_arya_HIConfigurationService>(pointer_to_object)
                , {} // don't provide access to IComponent interface
                , wrapper::arya::ConfigurationService::registerNode
                , wrapper::arya::ConfigurationService::unregisterNode
                , wrapper::arya::ConfigurationService::isNodeRegistered
                , wrapper::arya::ConfigurationService::getNode
                , wrapper::arya::ConfigurationService::getConstNode
                };
        }
        );
    }
    else
    {
        // Note: not an error, this function is just not capable of getting the component for the passed IID
        return fep3_plugin_c_interface_error_none;
    }
}

template<typename factory_type>
inline fep3_plugin_c_InterfaceError createConfigurationService
    (factory_type&& factory
    , fep3_arya_SIConfigurationService* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    using configuration_service_type = typename std::remove_pointer<decltype(std::declval<factory_type>()())>::type;
    if (0 == strcmp(configuration_service_type::getComponentIID(), iid))
    {
        return wrapper::arya::create
            (factory
            , result
            , shared_binary_access
            , [](configuration_service_type* pointer_to_object)
                {
                    return fep3_arya_SIConfigurationService
                        { reinterpret_cast<fep3_arya_HIConfigurationService>(static_cast<fep3::arya::IConfigurationService*>(pointer_to_object))
                        , wrapper::arya::Component::AccessCreator()(pointer_to_object)
                        , wrapper::arya::ConfigurationService::registerNode
                        , wrapper::arya::ConfigurationService::unregisterNode
                        , wrapper::arya::ConfigurationService::isNodeRegistered
                        , wrapper::arya::ConfigurationService::getNode
                        , wrapper::arya::ConfigurationService::getConstNode
                        };
                }
            );
    }
    else
    {
        // Note: not an error, this function is just not capable of creating the component for the passed IID
        return fep3_plugin_c_interface_error_none;
    }
}
/// @endcond no_documentation

} // namespace detail

/**
 * Creates a Configuration Service object of type \p configuration_service_type
 * @tparam configuration_service_type The type of the Configuration Service object to be created
 * @param[in,out] access_result Pointer to the access structure to the created Configuration Service object
 * @param[in] shared_binary_access Access strcuture to the shared binary the Configuration Service object resides in
 * @param[in] iid The interface ID of the Configuration Service interface of the created object
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The @p result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while creating the component
 */
template<typename configuration_service_type>
inline fep3_plugin_c_InterfaceError createConfigurationService
    (fep3_arya_SIConfigurationService* access_result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    return detail::createConfigurationService
    ([]()
    {
        return new configuration_service_type;
    }
        , access_result
        , shared_binary_access
        , iid
        );
}

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

ConfigurationService::ConfigurationService
    (Access&& access
    , const std::shared_ptr<c::arya::ISharedBinary>& shared_binary)
    : ::fep3::plugin::c::access::arya::ComponentBase<fep3::arya::IConfigurationService>
        (access._component
        , shared_binary)
        , _access(access)
{}

/// @cond no_documentation
fep3::Result ConfigurationService::registerNode(std::shared_ptr<fep3::arya::IPropertyNode> property_node)
{
    return arya::Helper::transferSharedPtrWithResultCallback<fep3::Result>
        (property_node
        , _access._handle
        , _access.registerNode
        , &getResult
        , fep3::plugin::c::wrapper::arya::PropertyNode<fep3_arya_HIPropertyNode>::AccessCreator()
        );
}

fep3::Result ConfigurationService::unregisterNode(const std::string& name)
{
    return arya::Helper::callWithResultCallback<fep3::Result>
        (_access._handle
        , _access.unregisterNode
        , &getResult
        , name.c_str()
    );
}

bool ConfigurationService::isNodeRegistered(const std::string& path) const
{
    return arya::Helper::callWithResultParameter
        (_access._handle
        , _access.isNodeRegistered
        , path.c_str()
    );
}

std::shared_ptr<fep3::arya::IPropertyNode> ConfigurationService::getNode(const std::string& path) const
{
    return arya::Helper::getSharedPtr<fep3::plugin::c::access::arya::PropertyNode<fep3_arya_SIPropertyNode>, fep3_arya_SIPropertyNode>
        (_access._handle
        , _access.getNode
        , path.c_str()
    );
}

std::shared_ptr<const fep3::arya::IPropertyNode> ConfigurationService::getConstNode(const std::string& path) const
{
    return arya::Helper::getSharedPtr<fep3::plugin::c::access::arya::PropertyNode<fep3_arya_const_SIPropertyNode>, fep3_arya_const_SIPropertyNode>
        (_access._handle
        , _access.getConstNode
        , path.c_str()
    );
}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3

/**
 * Gets access to a Configuration Service object as identified by @p handle_to_component
 * @param[in,out] access_result Pointer to the access structure to the Configuration Service object
 * @param[in] iid The interface ID of the Configuration Service interface to get
 * @param[in] handle_to_component Handle to the interface of the object to get
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle_to_component is null
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p access_result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while getting the component
 */
inline fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getConfigurationService
    (fep3_arya_SIConfigurationService* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    return ::fep3::plugin::c::wrapper::arya::detail::getConfigurationService
    (access_result
        , iid
        , handle_to_component
    );
}