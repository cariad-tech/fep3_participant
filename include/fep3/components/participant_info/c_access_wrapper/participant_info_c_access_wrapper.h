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
#include <fep3/components/participant_info/participant_info_intf.h>
#include <fep3/components/participant_info/c_intf/participant_info_c_intf.h>
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
 * @brief Access class for @ref fep3::arya::IParticipantInfo.
 * Use this class to access a remote object of a type derived from IParticipantInfo that resides in another binary (e. g. a shared library).
 */
class ParticipantInfo : public ::fep3::plugin::c::access::arya::ComponentBase<::fep3::arya::IParticipantInfo>
{
public:
    /// Symbol name of the create function that is capable to create a participant info
    static constexpr const char* const create_function_name = FEP3_EXPAND_TO_STRING(SYMBOL_fep3_plugin_c_arya_createParticipantInfo);
    /// Gets the function to get an instance of a participant info that resides in a C plugin
     static decltype(&fep3_plugin_c_arya_getParticipantInfo) getGetterFunction()
     {
          return fep3_plugin_c_arya_getParticipantInfo;
     }
    /// Type of access structure
    using Access = fep3_arya_SIParticipantInfo;

    /**
     * CTOR
     *
     * @param[in] access Access to the remote object
     * @param[in] shared_binary Shared pointer to the binary this resides in
     */
    inline ParticipantInfo(Access&& access, const std::shared_ptr<c::arya::ISharedBinary>& shared_binary);
    /**
     * DTOR destroying the corresponding remote object
     */
    inline ~ParticipantInfo() override = default;

    /// @cond no_documentation
    // methods implementing fep3::arya::IParticipantInfo
    inline std::string getName() const override;
    inline std::string getSystemName() const override;
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
 * Wrapper class for interface @ref fep3::arya::IParticipantInfo
 */
class ParticipantInfo : private arya::Helper<fep3::arya::IParticipantInfo>
{
private:
    using Helper = arya::Helper<fep3::arya::IParticipantInfo>;
    using Handle = fep3_arya_HIParticipantInfo;

public:
    /// @cond no_documentation
    static inline fep3_plugin_c_InterfaceError getName
        (Handle handle
        , void(*callback)(void*, const char*)
        , void* destination
        ) noexcept
    {
        return Helper::callWithResultCallback
            (handle
            , &fep3::arya::IParticipantInfo::getName
            , callback
            , destination
            , [](const std::string& name)
                {
                    return name.c_str();
                }
            );
    }

    static inline fep3_plugin_c_InterfaceError getSystemName
        (Handle handle
        , void(*callback)(void*, const char*)
        , void* destination
        ) noexcept
    {
        return Helper::callWithResultCallback
            (handle
            , &fep3::arya::IParticipantInfo::getSystemName
            , callback
            , destination
            , [](const std::string& system_name)
                {
                    return system_name.c_str();
                }
            );
    }
    /// @endcond no_documentation

private:
    /// Type of access structure
    using Access = fep3_arya_SIParticipantInfo;
};

namespace detail
{
/// @cond no_documentation
inline fep3_plugin_c_InterfaceError getParticipantInfo
    (fep3_arya_SIParticipantInfo* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    if (0 == strcmp(::fep3::arya::IParticipantInfo::getComponentIID(), iid))
    {
        return ::fep3::plugin::c::wrapper::arya::get<::fep3::arya::IComponent, ::fep3::arya::IParticipantInfo>
            (access_result
            , handle_to_component
            , [](::fep3::arya::IParticipantInfo* pointer_to_object)
        {
            return fep3_arya_SIParticipantInfo
            { reinterpret_cast<fep3_arya_HIParticipantInfo>(pointer_to_object)
            , {} // don't provide access to IComponent interface
            , wrapper::arya::ParticipantInfo::getName
            , wrapper::arya::ParticipantInfo::getSystemName
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
inline fep3_plugin_c_InterfaceError createParticipantInfo
    (factory_type&& factory
    , fep3_arya_SIParticipantInfo* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    using participant_info_type = typename std::remove_pointer<decltype(std::declval<factory_type>()())>::type;
    if (0 == strcmp(participant_info_type::getComponentIID(), iid))
    {
        return wrapper::arya::create
            (factory
            , result
            , shared_binary_access
            , [](participant_info_type* pointer_to_object)
                {
                    return fep3_arya_SIParticipantInfo
                        { reinterpret_cast<fep3_arya_HIParticipantInfo>(static_cast<fep3::arya::IParticipantInfo*>(pointer_to_object))
                        , wrapper::arya::Component::AccessCreator()(pointer_to_object)
                        , wrapper::arya::ParticipantInfo::getName
                        , wrapper::arya::ParticipantInfo::getSystemName
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
 * Creates a participant info object of type \p participant_info_type
 * @tparam participant_info_type The type of the participant info object to be created
 * @param[in,out] access_result Pointer to the access structure to the created participant info object
 * @param[in] shared_binary_access Access strcuture to the shared binary the participant info object resides in
 * @param[in] iid The interface ID of the participant info interface of the created object
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The @p result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while creating the component
 */
template<typename participant_info_type>
inline fep3_plugin_c_InterfaceError createParticipantInfo
    (fep3_arya_SIParticipantInfo* access_result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    return detail::createParticipantInfo
    ([]()
    {
        return new participant_info_type;
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

ParticipantInfo::ParticipantInfo
    (Access&& access
    , const std::shared_ptr<c::arya::ISharedBinary>& shared_binary)
    : ::fep3::plugin::c::access::arya::ComponentBase<fep3::arya::IParticipantInfo>
        (access._component
        , shared_binary)
        , _access(access)
{}

/// @cond no_documentation
std::string ParticipantInfo::getName() const
{
    return arya::Helper::callWithResultCallback<std::string>
        (_access._handle
        , _access.getName
        , [](auto result)
            {
                return result;
            }
        );
}

std::string ParticipantInfo::getSystemName() const
{
    return arya::Helper::callWithResultCallback<std::string>
        (_access._handle
        , _access.getSystemName
        , [](auto result)
    {
        return result;
    }
    );
}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3

/**
 * Gets access to a participant info object as identified by @p handle_to_component
 * @param[in,out] access_result Pointer to the access structure to the participant info object
 * @param[in] iid The interface ID of the participant info interface to get
 * @param[in] handle_to_component Handle to the interface of the object to get
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle_to_component is null
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p access_result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while getting the component
 */
inline fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getParticipantInfo
    (fep3_arya_SIParticipantInfo* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    return ::fep3::plugin::c::wrapper::arya::detail::getParticipantInfo
    (access_result
        , iid
        , handle_to_component
    );
}