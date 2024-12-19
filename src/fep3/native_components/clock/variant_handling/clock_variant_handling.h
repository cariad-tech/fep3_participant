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
#include <fep3/components/base/components_intf.h>
#include <fep3/components/clock/clock_intf.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/logging/logger_intf.h>

#include <functional>
#include <variant>

namespace fep3::native {

class ClockAdapterAryaToCatelyn : public fep3::experimental::IClock {
public:
    ClockAdapterAryaToCatelyn(std::shared_ptr<fep3::arya::IClock> clock);

    void start(const std::weak_ptr<fep3::experimental::IClock::IEventSink>& event_sink) override;

    void stop() override;

    std::string getName() const override;

    fep3::arya::IClock::ClockType getType() const override;
    fep3::arya::Timestamp getTime() const override;
    void reset(fep3::arya::Timestamp new_time) override;

private:
    std::shared_ptr<fep3::arya::IClock> _clock;
    std::shared_ptr<CatelynToAryaEventSinkAdapter> _event_sink_adapter;
};

// done so that the native clocks are also gettable as arya
class ClockAdapterCatelynToArya : public fep3::arya::IClock {
public:
    ClockAdapterCatelynToArya(std::shared_ptr<fep3::experimental::IClock> clock);

    void start(const std::weak_ptr<fep3::arya::IClock::IEventSink>& event_sink) override;

    void stop() override;

    std::string getName() const override;

    fep3::arya::IClock::ClockType getType() const override;
    fep3::arya::Timestamp getTime() const override;
    void reset(fep3::arya::Timestamp new_time) override;

private:
    std::shared_ptr<fep3::experimental::IClock> _clock;
    std::shared_ptr<AryaToCatelynEventSinkAdapter> _event_sink_adapter;
};

class ClockEventSinkRegistry;

class GenericClockAdapter {
    using ClockVariantType = std::variant<std::shared_ptr<fep3::arya::IClock>,
                                          std::shared_ptr<fep3::experimental::IClock>>;

public:
    template <typename T>
    GenericClockAdapter(std::shared_ptr<T> clock)
        : _clock(clock), _adapter(std::make_shared<CatelynToAryaEventSinkAdapter>())
    {
    }
    ~GenericClockAdapter();
    std::string getName() const;

    void start(const std::weak_ptr<fep3::experimental::IClock::IEventSink>& event_sink);

    void stop();

    fep3::arya::IClock::ClockType getType() const;

    arya::Timestamp getTime() const;

    void reset(fep3::arya::Timestamp new_time);

    template <typename T>
    std::shared_ptr<T> getClockPointer() const
    {
        return std::visit(
            [&](auto& arg) -> std::shared_ptr<T> {
                using ClockTypeInVariant =
                    typename std::remove_reference_t<decltype(arg)>::element_type;
                if constexpr (std::is_same_v<ClockTypeInVariant, T>)
                    return arg;
                else
                    return getClockAdapter(arg);
            },
            _clock);
    }

private:
    std::shared_ptr<fep3::experimental::IClock> getClockAdapter(
        std::shared_ptr<fep3::arya::IClock> clock) const;

    std::shared_ptr<fep3::arya::IClock> getClockAdapter(
        std::shared_ptr<fep3::experimental::IClock> clock) const;

private:
    ClockVariantType _clock;
    // adapter is saved here since the clock gets a weak ptr to arya:IEventSink
    // thus, not participating in the lifetime
    std::shared_ptr<CatelynToAryaEventSinkAdapter> _adapter;
};

template <typename T>
fep3::Result registerClockToService(std::shared_ptr<T> clock,
                                    const fep3::arya::IComponents& components,
                                    std::shared_ptr<ILogger> logger)
{
    using LatestClockService = ::fep3::experimental::IClockService;
    auto [res, clock_service] = fep3::base::getComponentHelper<LatestClockService>(components);

    if (clock_service && res) {
        return clock_service->registerClock(clock);
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
            return res_arya;
        }
        else {
            // we create an adapter to get an arya Clock
            GenericClockAdapter clock_adapter(clock);
            return clock_service_arya->registerClock(
                clock_adapter.getClockPointer<fep3::arya::IClock>());
        }
    }
}

fep3::Result unregisterClockFromService(const std::string& clock_name,
                                        const fep3::arya::IComponents& components,
                                        std::shared_ptr<fep3::arya::ILogger> logger);

} // namespace fep3::native
