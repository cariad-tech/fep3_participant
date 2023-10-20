/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include "local_scheduler_registry.h"

#include <fep3/components/job_registry/job_registry_intf.h>

#include <algorithm>
#include <iterator>

namespace fep3 {
namespace native {
fep3::Result LocalSchedulerRegistry::activateDefaultScheduler()
{
    return setActiveScheduler(_default_scheduler_name);
}

fep3::Result LocalSchedulerRegistry::setActiveScheduler(const std::string& scheduler_name)
{
    const auto scheduler = findScheduler(scheduler_name);
    if (!scheduler) {
        RETURN_ERROR_DESCRIPTION(
            ERR_NOT_FOUND,
            "Setting scheduler failed. A scheduler with the name '%s' is not registered.",
            scheduler_name.c_str());
    }
    _active_scheduler = scheduler_name;

    return {};
}

fep3::Result LocalSchedulerRegistry::registerScheduler(
    std::unique_ptr<fep3::arya::IScheduler> scheduler)
{
    return addSchedulerToList(std::move(scheduler));
}

fep3::Result LocalSchedulerRegistry::registerScheduler(
    std::unique_ptr<fep3::catelyn::IScheduler> scheduler)
{
    return addSchedulerToList(std::move(scheduler));
}

std::string LocalSchedulerRegistry::getActiveSchedulerName() const
{
    return _active_scheduler;
}

fep3::Result LocalSchedulerRegistry::deinitializeActiveScheduler()
{
    auto active_scheduler = getActiveScheduler();
    if (!active_scheduler) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "there is no active scheduler set");
    }

    return std::visit([](auto& scheduler) -> fep3::Result { return scheduler->deinitialize(); },
                      *active_scheduler);
}

fep3::Result LocalSchedulerRegistry::startActiveScheduler()
{
    auto active_scheduler = getActiveScheduler();
    if (!active_scheduler) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "there is no active scheduler set");
    }
    return std::visit([](auto& scheduler) -> fep3::Result { return scheduler->start(); },
                      *active_scheduler);
}

fep3::Result LocalSchedulerRegistry::stopActiveScheduler()
{
    auto active_scheduler = getActiveScheduler();
    if (!active_scheduler) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "there is no active scheduler set");
    }

    return std::visit([](auto& scheduler) -> fep3::Result { return scheduler->stop(); },
                      *active_scheduler);
}

std::string LocalSchedulerRegistry::getDefaultSchedulerName() const
{
    return _default_scheduler_name;
}

fep3::Result LocalSchedulerRegistry::initializeActiveScheduler(fep3::arya::IClockService& clock,
                                                               const fep3::arya::Jobs& jobs)
{
    auto active_scheduler = getActiveScheduler();
    if (!active_scheduler) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "there is no active scheduler set");
    }

    return std::visit(
        [&](auto& scheduler) -> fep3::Result { return scheduler->initialize(clock, jobs); },
        *active_scheduler);
}

fep3::Result LocalSchedulerRegistry::initializeActiveScheduler(const fep3::IComponents& components)
{
    auto active_scheduler = getActiveScheduler();
    if (!active_scheduler) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "there is no active scheduler set");
    }

    // we always need arya clock service for arya scheduler

    const auto job_registry = components.getComponent<fep3::IJobRegistry>();
    if (!job_registry) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "access to component IJobRegistry was not possible");
    }

    const auto jobs = job_registry->getJobs();

    return std::visit(
        [&](auto& arg) -> fep3::Result {
            using T = typename std::remove_reference_t<decltype(arg)>::element_type;
            if constexpr (std::is_same_v<T, fep3::arya::IScheduler>) {
                const auto clock_service = components.getComponent<fep3::arya::IClockService>();
                if (!clock_service) {
                    RETURN_ERROR_DESCRIPTION(ERR_NOT_FOUND,
                                             "scheduler initialization failed. %s is not part of "
                                             "the given component registry",
                                             getComponentIID<fep3::arya::IClockService>().c_str());
                }
                return arg->initialize(*clock_service, jobs);
            }
            else if constexpr (std::is_same_v<T, fep3::catelyn::IScheduler>)
                return arg->initialize(components);
            else
                RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG, "non-exhaustive visitor!");
            // static_assert(false, "non-exhaustive visitor!");
        },
        *active_scheduler);
}

LocalSchedulerRegistry::ISchedulerVariant* LocalSchedulerRegistry::getActiveScheduler()
{
    return findScheduler(_active_scheduler);
}

LocalSchedulerRegistry::ISchedulerVariant* LocalSchedulerRegistry::findScheduler(
    const std::string& scheduler_name)
{
    auto it = std::find_if(_schedulers.begin(), _schedulers.end(), [&](auto& scheduler_variant) {
        return std::visit([](auto& scheduler) -> std::string { return scheduler->getName(); },
                          scheduler_variant) == scheduler_name;
    });

    if (it != _schedulers.end()) {
        return &(*it);
    }
    else {
        return nullptr;
    }
}

fep3::Result LocalSchedulerRegistry::unregisterScheduler(const std::string& scheduler_name)
{
    if (_default_scheduler_name == scheduler_name) {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ARG,
                                 "Unregistering the default scheduler is not possible");
    }

    const auto it =
        std::remove_if(_schedulers.begin(), _schedulers.end(), [&](const auto& scheduler_variant) {
            return std::visit([&](auto& scheduler) -> std::string { return scheduler->getName(); },
                              scheduler_variant) == scheduler_name;
        });

    if (it == _schedulers.end()) {
        RETURN_ERROR_DESCRIPTION(
            ERR_NOT_FOUND,
            "Unregistering scheduler failed. A scheduler with the name '%s' is not registered.",
            scheduler_name.c_str());
    }

    _schedulers.erase(it);
    if (scheduler_name == getActiveSchedulerName()) {
        activateDefaultScheduler();
    }

    return {};
}

std::list<std::string> LocalSchedulerRegistry::getSchedulerNames() const
{
    std::list<std::string> scheduler_list;

    std::transform(
        _schedulers.begin(),
        _schedulers.end(),
        std::back_inserter(scheduler_list),
        [](const auto& scheduler_variant) {
            return std::visit([](auto& scheduler) -> std::string { return scheduler->getName(); },
                              scheduler_variant);
        }

    );

    return scheduler_list;
}

} // namespace native
} // namespace fep3
