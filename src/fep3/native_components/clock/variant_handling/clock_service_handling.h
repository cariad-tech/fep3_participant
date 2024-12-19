/**
 * Copyright 2024 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "clock_event_sink_variant_handling.h"

#include <fep3/components/base/component.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/logging/logger_intf.h>

#include <functional>
#include <memory>
#include <stdexcept>
#include <variant>

namespace fep3::native {

class ClockServiceAdapter {
    using ClockServiceVariantType =
        std::variant<fep3::arya::IClockService*, fep3::experimental::IClockService*>;

public:
    explicit ClockServiceAdapter(fep3::arya::IClockService* clock_service)
        : _clock_service(clock_service),
          _time_getter([clock_service]() { return clock_service->getTime(); })
    {
    }

    ~ClockServiceAdapter() = default;
    explicit ClockServiceAdapter(fep3::experimental::IClockService* clock_service)
        : _clock_service(clock_service),
          _time_getter([clock_service]() { return clock_service->getTime(); })
    {
    }

    fep3::Result registerEventSink(
        const std::weak_ptr<fep3::experimental::IClock::IEventSink>& clock_event_sink)
    {
        return std::visit(
            [&](auto arg) -> fep3::Result {
                using ClockServiceType = typename std::remove_pointer_t<decltype(arg)>;
                if constexpr (std::is_same_v<ClockServiceType, fep3::arya::IClockService>) {
                    _adapter = std::make_shared<CatelynToAryaEventSinkAdapter>(clock_event_sink);
                    return arg->registerEventSink(_adapter);
                }
                else
                    return arg->registerEventSink(clock_event_sink);
            },
            _clock_service);
    }

    fep3::Result unregisterEventSink(
        const std::weak_ptr<fep3::experimental::IClock::IEventSink>& clock_event_sink)
    {
        return std::visit(
            [&](auto arg) -> fep3::Result {
                using ClockServiceType = typename std::remove_pointer_t<decltype(arg)>;
                if constexpr (std::is_same_v<ClockServiceType, fep3::arya::IClockService>) {
                    // currenty supports only deregistering the sink that was registered.
                    assert(_adapter);
                    assert(_adapter->isEqual(clock_event_sink));
                    auto res = arg->unregisterEventSink(_adapter);
                    _adapter.reset();
                    return res;
                }
                else
                    return arg->unregisterEventSink(clock_event_sink);
            },
            _clock_service);
    }

    fep3::arya::IClock::ClockType getType()
    {
        return std::visit(
            [](auto& clock_service) -> fep3::arya::IClock::ClockType {
                return clock_service->getType();
            },
            _clock_service);
    }

    std::string getMainClockName() const
    {
        return std::visit(
            [](auto& clock_service) -> std::string { return clock_service->getMainClockName(); },
            _clock_service);
    }

    std::function<fep3::Timestamp()> getTimeGetter()
    {
        return _time_getter;
    }

private:
    ClockServiceVariantType _clock_service;
    std::shared_ptr<CatelynToAryaEventSinkAdapter> _adapter;
    std::function<fep3::Timestamp()> _time_getter;
};

inline std::pair<fep3::Result, std::unique_ptr<ClockServiceAdapter>> getClockServiceAdapter(
    fep3::arya::IClockService& clock_service, std::shared_ptr<const fep3::ILogger>)
{
    return {fep3::Result{}, std::make_unique<ClockServiceAdapter>(&clock_service)};
}

inline std::pair<fep3::Result, std::unique_ptr<ClockServiceAdapter>> getClockServiceAdapter(
    const fep3::IComponents& components, std::shared_ptr<const fep3::ILogger> logger)
{
    using LatestClockService = ::fep3::experimental::IClockService;
    auto [res, clock_service] = fep3::base::getComponentHelper<LatestClockService>(components);

    if (clock_service && res) {
        return {fep3::Result{}, std::make_unique<ClockServiceAdapter>(clock_service)};
    }
    else {
        if constexpr (std::is_same_v<LatestClockService, fep3::experimental::IClockService>) {
            // As long as interface is experimental the user does not want a log warning
            // that interface not found
            FEP3_LOGGER_LOG_DEBUG(logger, res.getDescription());
        }
        else {
            FEP3_LOGGER_LOG_WARNING(logger, res.getDescription());
        }

        // Fallback if latest version of interface IClockService is not available
        using AryaClockService = ::fep3::arya::IClockService;
        auto [res_arya, clock_service_arya] =
            fep3::base::getComponentHelper<AryaClockService>(components);
        if (!clock_service_arya || !res_arya) {
            FEP3_LOGGER_LOG_ERROR(
                logger,
                a_util::strings::format("Neither %s nor %s is part of the given component registry",
                                        AryaClockService::getComponentIID(),
                                        LatestClockService::getComponentIID()));
            return {res_arya, nullptr};
        }
        else {
            return {res_arya, std::make_unique<ClockServiceAdapter>(clock_service_arya)};
        }
    }
}

} // namespace fep3::native
