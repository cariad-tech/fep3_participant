/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/base/component.h>
#include <fep3/components/clock/clock_intf.h>
#include <fep3/components/clock_sync/clock_sync_service_intf.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/logging/logger_intf.h>
#include <fep3/components/service_bus/service_bus_intf.h>

namespace fep3 {

namespace rpc::arya {
class FarClockUpdater;
} // namespace rpc::arya

namespace native {

/**
 * @brief Configuration for the LocalClockService
 */
struct ClockSyncServiceConfiguration : public base::Configuration {
    ClockSyncServiceConfiguration();
    ~ClockSyncServiceConfiguration() = default;

    fep3::Result registerPropertyVariables() override;
    fep3::Result unregisterPropertyVariables() override;
    std::pair<bool, fep3::Result> validateConfiguration(
        const std::string& main_clock_name, const std::shared_ptr<fep3::ILogger>& logger) const;

    base::PropertyVariable<std::string> _timing_master_name{""};
    base::PropertyVariable<int64_t> _slave_sync_cycle_time{
        FEP3_SLAVE_SYNC_CYCLE_TIME_DEFAULT_VALUE};
};

/**
 * @brief Native implementation of a clock sync service.
 */
class ClockSynchronizationService : public base::Component<IClockSyncService>,
                                    public base::EasyLogging {
public:
    ClockSynchronizationService() = default;
    ~ClockSynchronizationService() = default;

public: // inherited via base::Component
    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result initialize() override;
    fep3::Result deinitialize() override;
    fep3::Result tense() override;

private:
    fep3::Result setupSlaveClock(const IComponents& components, const std::string& main_clock_name);
    std::pair<fep3::Result, std::shared_ptr<fep3::arya::IServiceBus::IParticipantServer>>
    getRpcServer(const IComponents& components) const;

private:
    template <typename T>
    struct SlaveClockAdapter {
        template <typename U>
        SlaveClockAdapter(std::shared_ptr<U> clock) : _logger(clock), _slave_clock(clock)
        {
        }

        // SlaveClockAdapter(SlaveClockAdapter&&) = default;
        // SlaveClockAdapter& operator=(SlaveClockAdapter&&) = default;

        ~SlaveClockAdapter()
        {
            _logger->deinitLogger();
        }

        std::shared_ptr<T> get()
        {
            return _slave_clock;
        }

        std::shared_ptr<fep3::base::EasyLogging> logger()
        {
            return _logger;
        }

    private:
        std::shared_ptr<fep3::base::EasyLogging> _logger;
        std::shared_ptr<T> _slave_clock;
    };
    std::unique_ptr<SlaveClockAdapter<fep3::experimental::IClock>> _slave_clock;
    std::unique_ptr<SlaveClockAdapter<fep3::rpc::arya::FarClockUpdater>> _rpc_clock_sync_slave;

    ClockSyncServiceConfiguration _configuration;
};

} // namespace native
} // namespace fep3
