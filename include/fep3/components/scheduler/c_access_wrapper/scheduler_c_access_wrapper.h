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

#include <fep3/components/scheduler/c_intf/scheduler_c_intf.h>
#include <fep3/components/scheduler/scheduler_intf.h>
#include <fep3/components/job_registry/job_info.h>
#include <fep3/components/job_registry/job_configuration.h>
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
#include <fep3/components/job_registry/c_access_wrapper/job_c_access_wrapper.h>
#include <fep3/components/clock/c_access_wrapper/clock_service_c_access_wrapper.h>

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
 * Access class for @ref fep3::arya::IScheduler.
 * Use this class to access a remote object of a type derived from @ref fep3::arya::IScheduler
 * that resides in another binary (e. g. a shared library).
 */
class Scheduler
    : public ::fep3::arya::IScheduler
    , private c::arya::DestructionManager
    , private arya::Helper
{
public:
    /// Type of access structure
    using Access = fep3_arya_SIScheduler;

    /**
     * @brief CTOR
     * @param[in] access Access to the remote object
     * @param[in] destructors List of destructors to be called upon destruction of this
     */
    inline Scheduler
        (const Access& access
        , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
        );
    inline ~Scheduler() override = default;

    /// @cond no_documentation
    // methods implementing fep3::arya::IScheduler
    inline std::string getName() const override;
    inline fep3::Result initialize
        (fep3::arya::IClockService& clock
        , const fep3::arya::Jobs& jobs
        ) override;
    inline fep3::Result start() override;
    inline fep3::Result stop() override;
    inline fep3::Result deinitialize() override;
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
 * Wrapper class for interface @ref fep3::arya::IScheduler
 */
class Scheduler : private arya::Helper<fep3::arya::IScheduler>
{
private:
    /// Alias for the helper
    using Helper = arya::Helper<fep3::arya::IScheduler>;
    /// Alias for the type of the handle to a wrapped object of type @ref fep3::arya::IScheduler
    using Handle = fep3_arya_HIScheduler;

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
            , &fep3::arya::IScheduler::getName
            , callback
            , destination
            , [](const std::string& name)
                {
                    return name.c_str();
                }
            );
    }

    static inline fep3_plugin_c_InterfaceError initialize
        (Handle handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        , fep3_arya_SIClockService clock_service_access
        , void(*callback)(const void*, void(*)(void*, const char*, fep3_arya_SJobEntry), void*)
        , const void* jobs_source
        ) noexcept
    {
        fep3::arya::Jobs jobs;
        // pass a callback to the callback for filling "jobs"
        callback
            (jobs_source
            , [](void* destination, const char* name, fep3_arya_SJobEntry job_entry_access)
                {
                    const auto& job_info_access = job_entry_access._job_info;

                    std::deque<std::unique_ptr<c::arya::IDestructor>> job_destructors;
                    // shared ownership: release reference to remote object when local object is destroyed
                    job_destructors.push_back(std::make_unique<access::arya::Destructor<fep3_plugin_c_arya_SDestructionManager>>(job_entry_access._job_reference_manager));

                    fep3::arya::Jobs& jobs = *static_cast<fep3::arya::Jobs*>(destination);
                    jobs.emplace
                        (name
                        , fep3::arya::JobEntry
                            {std::make_shared<fep3::plugin::c::access::arya::Job>
                                (job_entry_access._job
                                , std::move(job_destructors)
                                )
                            , fep3::arya::JobInfo
                                (access::arya::fetchJobName(job_info_access)
                                , access::arya::createJobConfiguration(access::arya::fetchJobConfigurationAccess(job_info_access))
                                )
                            }
                        );
                }
            , &jobs
            );

        return passReferenceWithResultCallback<access::arya::ClockService>
            (handle
            , [](auto&& scheduler, auto&& clock_service, auto&& jobs)
                {
                    return scheduler->initialize
                        (std::forward<decltype(clock_service)>(clock_service)
                        , std::forward<decltype(jobs)>(jobs)
                        );
                }
            , result_callback
            , result_destination
            , getResult
            , clock_service_access
            , jobs
            );
    }

    static inline fep3_plugin_c_InterfaceError start
        (Handle handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        ) noexcept
    {
        return callWithResultCallback
            (handle
            , &fep3::arya::IScheduler::start
            , result_callback
            , result_destination
            , getResult
            );
    }

    static inline fep3_plugin_c_InterfaceError stop
        (Handle handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        ) noexcept
    {
        return callWithResultCallback
            (handle
            , &fep3::arya::IScheduler::stop
            , result_callback
            , result_destination
            , getResult
            );
    }

    static inline fep3_plugin_c_InterfaceError deinitialize
        (Handle handle
        , fep3_result_callback_type result_callback
        , void* result_destination
        ) noexcept
    {
        return callWithResultCallback
            (handle
            , &fep3::arya::IScheduler::deinitialize
            , result_callback
            , result_destination
            , getResult
            );
    }
    /// @endcond no_documentation
};

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

Scheduler::Scheduler
    (const Access& access
    , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
    )
    : _access(access)
{
    addDestructors(std::move(destructors));
}

/// @cond no_documentation
std::string Scheduler::getName() const
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

fep3::Result Scheduler::initialize
    (fep3::arya::IClockService& clock
    , const fep3::arya::Jobs& jobs
    )
{
    auto pointer_to_clock_service = &clock;
    fep3::Result result{};
    fep3_plugin_c_InterfaceError error = _access.initialize
        (_access._handle
        , [](void* destination, fep3_SResult result)
            {
                *static_cast<fep3::Result*>(destination) = getResult(result);
            }
        , &result
        , fep3_arya_SIClockService
            {reinterpret_cast<fep3_arya_HIClockService>(pointer_to_clock_service)
            , fep3_arya_SIComponent{} // note access to component interface disabled by intent
            , wrapper::arya::ClockService::getTime
            , wrapper::arya::ClockService::getTimeByClockName
            , wrapper::arya::ClockService::getType
            , wrapper::arya::ClockService::getTypeByClockName
            , wrapper::arya::ClockService::getMainClockName
            , wrapper::arya::ClockService::registerEventSink
            , wrapper::arya::ClockService::unregisterEventSink
            , wrapper::arya::ClockService::registerClock
            , wrapper::arya::ClockService::unregisterClock
            , wrapper::arya::ClockService::getClockNames
            , wrapper::arya::ClockService::findClock
            }
        , []
            (const void* jobs_source
            , void(*callback)(void*, const char*, fep3_arya_SJobEntry)
            , void* jobs_destination
            )
            {
                const ::fep3::arya::Jobs& jobs = *static_cast<const fep3::arya::Jobs*>(jobs_source);
                for(const auto& job_element : jobs)
                {
                    const auto& job_name = job_element.first;
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
                        (jobs_destination
                        , job_name.c_str()
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
                                        callback(destination, wrapper::arya::Job::JobConfigurationAccessCreator()(*job_info_source));
                                    }
                                , &job_info
                               }
                            }
                        );
                }
            }
        , static_cast<const void*>(&jobs)
        );
    if(fep3_plugin_c_interface_error_none != error)
    {
        throw Exception(error);
    }
    return result;
}

fep3::Result Scheduler::start()
{
    return arya::Helper::callWithResultCallback<fep3::Result>
        (_access._handle
        , _access.start
        , &getResult
        );
}

fep3::Result Scheduler::stop()
{
    return arya::Helper::callWithResultCallback<fep3::Result>
        (_access._handle
        , _access.stop
        , &getResult
        );
}

fep3::Result Scheduler::deinitialize()
{
    return arya::Helper::callWithResultCallback<fep3::Result>
        (_access._handle
        , _access.deinitialize
        , &getResult
        );
}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
