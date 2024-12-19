/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/scheduler/scheduler_intf.h>
#include <fep3/components/scheduler/scheduler_registry_intf.h>

#include <cassert>
#include <variant>

namespace fep3 {

namespace native {

class LocalSchedulerRegistry : public ISchedulerRegistry, public fep3::arya::ISchedulerRegistry {
public:
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<fep3::arya::IScheduler, T>>>
    LocalSchedulerRegistry(std::unique_ptr<T> default_scheduler)
        : _default_scheduler_name(default_scheduler->getName()), _active_scheduler("")
    {
        registerScheduler(std::move(default_scheduler));
        const auto result = activateDefaultScheduler();
        if (!result) {
            throw std::runtime_error(result.getDescription());
        }

        assert(!_active_scheduler.empty());
    }

    LocalSchedulerRegistry() = delete;

public:
    // ISchedulerRegistry
    fep3::Result registerScheduler(
        std::unique_ptr<fep3::arya::IScheduler> scheduler) override final;
    fep3::Result registerScheduler(
        std::unique_ptr<fep3::catelyn::IScheduler> scheduler) override final;

    fep3::Result unregisterScheduler(const std::string& scheduler_name) override final;
    std::list<std::string> getSchedulerNames() const override final;

    fep3::Result setActiveScheduler(const std::string& scheduler_name);
    std::string getActiveSchedulerName() const;
    std::string getDefaultSchedulerName() const;

    fep3::Result initializeActiveScheduler(fep3::arya::IClockService& clock,
                                           const fep3::arya::Jobs& jobs);

    fep3::Result initializeActiveScheduler(const fep3::IComponents& components);

    fep3::Result deinitializeActiveScheduler();
    fep3::Result startActiveScheduler();
    fep3::Result stopActiveScheduler();

private:
    using ISchedulerVariant = std::variant<std::unique_ptr<fep3::arya::IScheduler>,
                                           std::unique_ptr<fep3::catelyn::IScheduler>>;

    template <typename T, typename = std::enable_if_t<std::is_base_of_v<fep3::arya::IScheduler, T>>>
    fep3::Result addSchedulerToList(std::unique_ptr<T> scheduler)
    {
        const auto find_result = findScheduler(scheduler->getName());
        if (find_result != nullptr) {
            RETURN_ERROR_DESCRIPTION(ERR_RESOURCE_IN_USE,
                                     "Registering scheduler failed. A scheduler with the name '%s' "
                                     "is already registered.",
                                     scheduler->getName().c_str());
        }

        _schedulers.push_back(std::move(scheduler));
        return {};
    }

    ISchedulerVariant* getActiveScheduler();
    ISchedulerVariant* findScheduler(const std::string& scheduler_name);
    fep3::Result activateDefaultScheduler();

private:
    std::string _default_scheduler_name;
    std::string _active_scheduler;
    std::list<ISchedulerVariant> _schedulers;
};

} // namespace native
} // namespace fep3
