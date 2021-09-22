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
#include <functional>

#include <a_util/strings.h>

#include <fep3/fep3_macros.h>
#include <fep3/components/job_registry/c_intf/job_registry_c_intf.h>
#include <fep3/components/job_registry/job_registry_intf.h>
#include <fep3/components/base/c_access_wrapper/component_base_c_access.h>
#include <fep3/components/base/c_access_wrapper/component_c_wrapper.h>
#include <fep3/components/job_registry/c_access_wrapper/job_c_access_wrapper.h>

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
 * @brief Access class for @ref fep3::arya::IJobRegistry.
 * Use this class to access a remote object of a type derived from IJobRegistry that resides in another binary (e. g. a shared library).
 */
class JobRegistry
    : public fep3::plugin::c::access::arya::ComponentBase<fep3::arya::IJobRegistry>
{
public:
    /// Symbol name of the create function that is capable to create a scheduler service
    static constexpr const char* const create_function_name = FEP3_EXPAND_TO_STRING(SYMBOL_fep3_plugin_c_arya_createJobRegistry);
    /// Gets the function to get an instance of a job registry that resides in a C plugin
    static decltype(&fep3_plugin_c_arya_getJobRegistry) getGetterFunction()
    {
         return fep3_plugin_c_arya_getJobRegistry;
    }
    /// Type of access object
    using Access = fep3_arya_SIJobRegistry;

    /**
     * CTOR
     *
     * @param[in] access Access to the remote object
     * @param[in] shared_binary Shared pointer to the binary this resides in
     */
    inline JobRegistry
        (const Access& access
        , const std::shared_ptr<c::arya::ISharedBinary>& shared_binary
        );
    /**
     * DTOR destroying the corresponding remote object
     */
    inline ~JobRegistry() override = default;

    // methods implementing fep3::arya::IJobRegistry
    /// @cond no_documentation
    inline fep3::Result addJob
        (const std::string& name
        , const std::shared_ptr<fep3::arya::IJob>& job
        , const fep3::arya::JobConfiguration& job_config
        ) override;
    inline fep3::Result removeJob(const std::string& name) override;
    inline std::list<fep3::arya::JobInfo> getJobInfos() const override;
    inline Jobs getJobs() const override;
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
 * Wrapper class for interface \ref fep3::arya::IJobRegistry
 */
class JobRegistry : private arya::Helper<fep3::arya::IJobRegistry>
{
private:
    using Helper = arya::Helper<fep3::arya::IJobRegistry>;
    using Handle = fep3_arya_HIJobRegistry;

public:
    /// @cond no_documentation
    static inline fep3_plugin_c_InterfaceError addJob
        (Handle handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        , fep3_plugin_c_arya_SDestructionManager job_destruction_manager
        , fep3_arya_SIJob job_access
        , const char* name
        , fep3_arya_SJobConfiguration job_configuration_access
        ) noexcept
    {
        return transferSharedPtrWithResultCallback<access::arya::Job>
            (handle
            , [](auto&& job_registry, auto&& job, auto&& name, auto&& job_config)
                {
                    // attention: re-reordering arguments job and name to original order
                    return job_registry->addJob
                        (std::forward<decltype(name)>(name)
                        , std::forward<decltype(job)>(job)
                        , std::forward<decltype(job_config)>(job_config)
                        );
                }
            , result_callback
            , result_destination
            , &getResult
            , job_destruction_manager
            // attention: reordering arguments job and name to be able to use the helper method
            , job_access
            , name
            , access::arya::createJobConfiguration(job_configuration_access)
            );
    }

    static inline fep3_plugin_c_InterfaceError removeJob
        (Handle handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        , const char* name
        ) noexcept
    {
        return callWithResultCallback
            (handle
            , &fep3::arya::IJobRegistry::removeJob
            , result_callback
            , result_destination
            , &getResult
            , name
            );
    }

    static inline fep3_plugin_c_InterfaceError getJobInfos
        (Handle handle
        , void(*callback)(void*, fep3_arya_SJobInfo)
        , void* destination
        ) noexcept
    {
        return callWithRecurringResultCallback
            (handle
            , &fep3::arya::IJobRegistry::getJobInfos
            , callback
            , destination
            , [](const fep3::arya::JobInfo& job_info)
                {
                    return fep3_arya_SJobInfo
                        {[](const void* source, void(*callback)(void*, const char*), void* destination)
                            {
                                auto job_info_source = static_cast<const fep3::arya::JobInfo*>(source);
                                callback(destination, job_info_source->getName().c_str());
                            }
                        , [](const void* source, void(*callback)(void*, fep3_arya_SJobConfiguration), void* destination)
                            {
                                auto job_info_source = static_cast<const fep3::arya::JobInfo*>(source);
                                callback(destination, arya::Job::JobConfigurationAccessCreator()(*job_info_source));
                            }
                        , &job_info
                        };
                }
            );
    }

    static inline fep3_plugin_c_InterfaceError getJobs
        (Handle handle
        , void(*callback)(void*, fep3_arya_SJobEntry)
        , void* destination
        ) noexcept
    {
        try
        {
            if(const auto& pointer_to_object = reinterpret_cast<fep3::arya::IJobRegistry*>(handle))
            {
                const auto& jobs = pointer_to_object->getJobs();
                for(const auto& job_element : jobs)
                {
                    const auto& job_entry = job_element.second;
                    const auto& job = job_entry.job;
                    const auto& job_info = job_entry.job_info;
                    auto job_reference_manager = new c::arya::DestructionManager;
                    // reference to the local job must be released when the remote job is destroyed, so we add a (new) shared reference to the reference manager
                    job_reference_manager->addDestructor
                        (std::make_unique<c::arya::OtherDestructor<typename std::remove_reference<decltype(job)>::type>>
                        (new std::shared_ptr<typename std::decay<decltype(job)>::type::element_type>(job)));
                    auto job_reference_manager_access = fep3_plugin_c_arya_SDestructionManager
                        {reinterpret_cast<fep3_plugin_c_arya_HDestructionManager>(static_cast<c::arya::DestructionManager*>(job_reference_manager))
                        , wrapper::arya::Destructor::destroy
                        };
                    callback
                        (destination
                        , fep3_arya_SJobEntry
                            {job_reference_manager_access
                            , fep3_arya_SIJob
                                {reinterpret_cast<fep3_arya_HIJob>(job.get())
                                , wrapper::arya::Job::executeDataIn
                                , wrapper::arya::Job::execute
                                , wrapper::arya::Job::executeDataOut
                                }
                            , fep3_arya_SJobInfo
                                {[](const void* source, void(*callback)(void*, const char*), void* destination)
                                    {
                                        auto job_info_source = static_cast<const fep3::arya::JobInfo*>(source);
                                        callback(destination, job_info_source->getName().c_str());
                                    }
                                , [](const void* source, void(*callback)(void*, fep3_arya_SJobConfiguration), void* destination)
                                    {
                                        auto job_info_source = static_cast<const fep3::arya::JobInfo*>(source);
                                        callback(destination, arya::Job::JobConfigurationAccessCreator()(*job_info_source));
                                    }
                                , &job_info
                                }
                            }
                        );
                }
                return fep3_plugin_c_interface_error_none;
            }
            else
            {
                return fep3_plugin_c_interface_error_invalid_handle;
            }
        }
        catch(...)
        {
            return fep3_plugin_c_interface_error_exception_caught;
        }
    }
    /// @endcond no_documentation

public:
    /// Type of access structure
    using Access = fep3_arya_SIJobRegistry;
};

namespace detail
{

/// @cond no_documentation
inline fep3_plugin_c_InterfaceError getJobRegistry
    (fep3_arya_SIJobRegistry* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    if(0 == strcmp(fep3::arya::IJobRegistry::getComponentIID(), iid))
    {
        return fep3::plugin::c::wrapper::arya::get<fep3::arya::IComponent, fep3::arya::IJobRegistry>
            (access_result
            , handle_to_component
            , [](fep3::arya::IJobRegistry* pointer_to_object)
                {
                    return fep3_arya_SIJobRegistry
                        {reinterpret_cast<fep3_arya_HIJobRegistry>(pointer_to_object)
                        , {} // don't provide access to IComponent interface
                        , wrapper::arya::JobRegistry::addJob
                        , wrapper::arya::JobRegistry::removeJob
                        , wrapper::arya::JobRegistry::getJobInfos
                        , wrapper::arya::JobRegistry::getJobs
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
fep3_plugin_c_InterfaceError createJobRegistry
    (factory_type&& factory
    , fep3_arya_SIJobRegistry* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    using job_registry_type = typename std::remove_pointer<decltype(std::declval<factory_type>()())>::type;
    if(0 == strcmp(job_registry_type::getComponentIID(), iid))
    {
        return create
            (factory
            , result
            , shared_binary_access
            , [](job_registry_type* pointer_to_object)
                {
                    return fep3_arya_SIJobRegistry
                        {reinterpret_cast<fep3_arya_HIJobRegistry>(static_cast<fep3::arya::IJobRegistry*>(pointer_to_object))
                        , fep3::plugin::c::wrapper::arya::Component::AccessCreator()(pointer_to_object)
                        , wrapper::arya::JobRegistry::addJob
                        , wrapper::arya::JobRegistry::removeJob
                        , wrapper::arya::JobRegistry::getJobInfos
                        , wrapper::arya::JobRegistry::getJobs
                        };
                }
            );
    }
    else
    {
        // Note: not an error, this plugin is just not capable of creating the component for the passed IID
        return fep3_plugin_c_interface_error_none;
    }
}
/// @endcond no_documentation

} // namespace detail

/**
 * Creates a job registry object of type \p job_registry_type
 * @tparam The type of the job registry object to be created
 * @param[in,out] result Pointer to the access structure to the created job registry object
 * @param[in] shared_binary_access Access strcuture to the shared binary the job registry object resides in
 * @param[in] iid The interface ID of the job registry interface of the created object
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The @p result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while creating the component
 */
template<typename job_registry_type>
fep3_plugin_c_InterfaceError createJobRegistry
    (fep3_arya_SIJobRegistry* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    return detail::createJobRegistry
        ([]()
            {
                return new job_registry_type;
            }
        , result
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

JobRegistry::JobRegistry
    (const Access& access
    , const std::shared_ptr<c::arya::ISharedBinary>& shared_binary
    )
    : fep3::plugin::c::access::arya::ComponentBase<fep3::arya::IJobRegistry>(access._component, shared_binary)
    , _access(access)
{}

/// @cond no_documentation
fep3::Result JobRegistry::addJob(const std::string& name, const std::shared_ptr<fep3::arya::IJob>& job, const fep3::arya::JobConfiguration& job_config)
{
    return arya::Helper::transferSharedPtrWithResultCallback<fep3::Result>
        (job
        , _access._handle
        , _access.addJob
        , &getResult
        , [](const auto& pointer_to_job)
            {
                return fep3_arya_SIJob
                    {reinterpret_cast<fep3_arya_HIJob>(pointer_to_job)
                    , wrapper::arya::Job::executeDataIn
                    , wrapper::arya::Job::execute
                    , wrapper::arya::Job::executeDataOut
                    };
            }
        , name.c_str()
        , wrapper::arya::Job::JobConfigurationAccessCreator()(job_config)
        );
}

fep3::Result JobRegistry::removeJob(const std::string& name)
{
    return arya::Helper::callWithResultCallback<fep3::Result>
        (_access._handle
        , _access.removeJob
        , &getResult
        , name.c_str()
        );
}

std::list<fep3::arya::JobInfo> JobRegistry::getJobInfos() const
{
    return arya::Helper::callWithRecurringResultCallback<std::list<fep3::arya::JobInfo>, fep3_arya_SJobInfo>
        (_access._handle
        , _access.getJobInfos
        , [](const fep3_arya_SJobInfo& job_info_access)
            {
                return fep3::arya::JobInfo
                    (arya::fetchJobName(job_info_access)
                    , arya::createJobConfiguration(fetchJobConfigurationAccess(job_info_access))
                    );
            }
        , &std::list<fep3::arya::JobInfo>::push_back
        );
}

fep3::arya::Jobs JobRegistry::getJobs() const
{
    return arya::Helper::callWithRecurringResultCallback
        <fep3::arya::Jobs
        , fep3_arya_SJobEntry
        , std::pair<fep3::arya::Jobs::iterator, bool>
        >
        (_access._handle
        , _access.getJobs
        , [](const fep3_arya_SJobEntry& job_entry_access)
            {
                const auto& job_info_access = job_entry_access._job_info;

                std::deque<std::unique_ptr<c::arya::IDestructor>> job_destructors;
                // shared ownership: release reference to remote object when local object is destroyed
                job_destructors.push_back(std::make_unique<access::arya::Destructor<fep3_plugin_c_arya_SDestructionManager>>(job_entry_access._job_reference_manager));

                const auto& job_name = arya::fetchJobName(job_info_access);
                const auto& job_configuration_access = arya::fetchJobConfigurationAccess(job_info_access);

                return std::make_pair
                    (job_name
                    , fep3::arya::JobEntry
                        {std::make_shared<access::arya::Job>
                            (job_entry_access._job
                            , std::move(job_destructors)
                            )
                        , fep3::arya::JobInfo
                            (job_name
                            , access::arya::createJobConfiguration(job_configuration_access)
                            )
                        }
                    );
            }
        , &fep3::arya::Jobs::emplace
        );
}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3

/**
 * Gets access to a job registry object as identified by @p handle_to_component
 * @param[in,out] access_result Pointer to the access structure to the job registry object
 * @param[in] iid The interface ID of the job registry interface to get
 * @param[in] handle_to_component Handle to the interface of the object to get
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p access_result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while getting the component
 */
inline fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getJobRegistry
    (fep3_arya_SIJobRegistry* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    return fep3::plugin::c::wrapper::arya::detail::getJobRegistry
        (access_result
        , iid
        , handle_to_component
        );
}
