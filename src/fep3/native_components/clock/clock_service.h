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

#pragma once

#include "clock_event_sink_registry.h"
#include "clock_main_event_sink.h"
#include "clock_service_configuration.h"

#include <fep3/base/properties/propertynode.h>
#include <fep3/components/base/component.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/logging/logging_service_intf.h>
#include <fep3/fep3_optional.h>
#include <fep3/native_components/clock/clock_registry.h>
#include <fep3/native_components/clock/simulation_clock.h>
#include <fep3/native_components/clock/system_clock.h>

#include <list>
#include <memory>
#include <mutex>
#include <string>

namespace fep3::arya {
class IServiceBus;
class IRPCServer;
} // namespace fep3::arya

namespace fep3 {
namespace rpc {
class RPCClockSyncService;
class RPCClockService;
} // namespace rpc

namespace native {
using rpc::RPCClockService;
using rpc::RPCClockSyncService;

class GenericClockAdapter;
/**
 * @brief Native implementation of a clock service.
 */
class ClockService
    : public base::Component<fep3::arya::IClockService, fep3::experimental::IClockService>,
      public base::EasyLogging {
public:
    ClockService();
    ~ClockService() = default;

public: // inherited via IClockService
    Timestamp getTime() const override final;
    Optional<Timestamp> getTime(const std::string& clock_name) const override final;
    fep3::arya::IClock::ClockType getType() const override final;
    Optional<fep3::arya::IClock::ClockType> getType(
        const std::string& clock_name) const override final;
    std::string getMainClockName() const override final;

    fep3::Result registerEventSink(
        const std::weak_ptr<fep3::arya::IClock::IEventSink>& clock_event_sink) override final;
    fep3::Result unregisterEventSink(
        const std::weak_ptr<fep3::arya::IClock::IEventSink>& clock_event_sink) override final;

public: // inherited via IClockRegistry
    fep3::Result registerClock(const std::shared_ptr<fep3::arya::IClock>& clock) override final;
    fep3::Result unregisterClock(const std::string& clock_name) override final;
    std::list<std::string> getClockNames() const override final;
    std::shared_ptr<fep3::arya::IClock> findClock(
        const std::string& clock_name) const override final;

    // catelyn
    fep3::Result registerEventSink(
        const std::weak_ptr<fep3::experimental::IClock::IEventSink>& clock_event_sink) override;

    fep3::Result unregisterEventSink(
        const std::weak_ptr<fep3::experimental::IClock::IEventSink>& clock_event_sink) override;

    std::shared_ptr<experimental::IClock> findClockCatelyn(
        const std::string& clock_name) const override;

    virtual fep3::Result registerClock(
        const std::shared_ptr<fep3::experimental::IClock>& clock) override final;

public: // inherited via base::Component
    fep3::Result create() override;
    fep3::Result destroy() override;
    fep3::Result initialize() override;
    fep3::Result tense() override;
    fep3::Result relax() override;
    fep3::Result start() override;
    fep3::Result stop() override;

private:
    template <typename T>
    fep3::Result registerClockImpl(std::shared_ptr<T> clock);

    template <typename T>
    fep3::Result registerEventSinkImpl(const std::weak_ptr<T>& clock_event_sink);

    template <typename T>
    fep3::Result unregisterEventSinkImpl(const std::weak_ptr<T>& clock_event_sink);

    fep3::Result setupLogger(const IComponents& components);
    fep3::Result setupRPCLogger(const IComponents& components);
    fep3::Result unregisterServices(const IComponents& components) const;
    fep3::Result registerDefaultClocks();
    fep3::Result setupClockMainEventSink(const fep3::arya::IServiceBus& service_bus);
    fep3::Result setupRPCClockSyncService(fep3::arya::IRPCServer& rpc_server);
    fep3::Result setupRPCClockService(fep3::arya::IRPCServer& rpc_server);
    fep3::Result selectMainClock(const std::string& clock_name);

    template <typename T>
    std::invoke_result_t<T, GenericClockAdapter&> performLockedOp(T op)
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

        return op(_current_clock);
    }

    template <typename T>
    std::invoke_result_t<T, const GenericClockAdapter&> performLockedOp(T op) const
    {
        std::lock_guard<std::recursive_mutex> lock_guard(_recursive_mutex);

        return op(_current_clock);
    }

private:
    mutable std::recursive_mutex _recursive_mutex;
    std::mutex _select_main_clock_mutex;

    std::unique_ptr<ClockRegistry> _clock_registry;
    ClockServiceConfiguration _configuration;

    std::atomic<bool> _is_started;
    std::atomic<bool> _is_tensed;

    std::shared_ptr<SystemClock> _realtime_clock;
    std::shared_ptr<SimulationClock> _simulation_clock;
    GenericClockAdapter _current_clock;

    std::shared_ptr<ClockEventSinkRegistry> _clock_event_sink_registry;

    std::shared_ptr<RPCClockSyncService> _rpc_clock_sync_service{nullptr};
    std::shared_ptr<rpc::ClockMainEventSink> _clock_main_event_sink;
    std::shared_ptr<RPCClockService> _rpc_clock_service{nullptr};
};

} // namespace native
} // namespace fep3
