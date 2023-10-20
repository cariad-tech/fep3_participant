/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once
#include <fep3/components/base/component_intf.h>
#include <fep3/components/base/components_intf.h>
#include <fep3/components/clock/clock_intf.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/logging/logger_intf.h>
#include <fep3/fep3_participant_version.h>

#include <functional>
#include <variant>

namespace fep3::native {

class CatelynToAryaEventSinkAdapter;
class AryaToCatelynEventSinkAdapter;

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
    GenericClockAdapter(std::shared_ptr<T> clock) : _clock(clock)
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
                                    std::shared_ptr<ILogger>)
{
    auto clock_service_catelyn = components.getComponent<fep3::experimental::IClockService>();

    if (clock_service_catelyn) {
        return clock_service_catelyn->registerClock(clock);
    }
    else {
#if FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR > 2
        FEP3_ARYA_LOGGER_LOG_WARNING(
            logger,
            a_util::strings::format(
                "%s is not part of the given component registry, it is recommended to"
                "load FEP Component Implementation %s",
                fep3::experimental::IClockService::getComponentIID(),
                fep3::experimental::IClockService::getComponentIID()));
#endif // FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR > 2
        auto clock_service_arya = components.getComponent<fep3::arya::IClockService>();
        // no clock service available
        if (!clock_service_arya) {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_NO_INTERFACE,
                                     "Neither %s or %s are part of the given component registry",
                                     fep3::arya::IClockService::getComponentIID(),
                                     fep3::experimental::IClockService::getComponentIID());
        }
        // only arya clock service available
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
