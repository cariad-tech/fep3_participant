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

#include "clock_event_sink_variant_handling.h"

#include <fep3/components/base/components_intf.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/easy_logger.h>
#include <fep3/components/logging/logger_intf.h>
#include <fep3/fep3_participant_version.h>

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
    auto clock_service_catelyn = components.getComponent<fep3::experimental::IClockService>();

    if (clock_service_catelyn) {
        return {fep3::Result{}, std::make_unique<ClockServiceAdapter>(clock_service_catelyn)};
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
#endif // FEP3_PARTICIPANT_LIBRARY_VERSION_MINOR >
        auto clock_service_arya = components.getComponent<fep3::arya::IClockService>();
        if (clock_service_arya) {
            return {fep3::Result{}, std::make_unique<ClockServiceAdapter>(clock_service_arya)};
        }
        else {
            auto error_desc = CREATE_ERROR_DESCRIPTION(
                fep3::ERR_NO_INTERFACE,
                "Neither %s or %s are part of the given component registry",
                fep3::arya::IClockService::getComponentIID(),
                fep3::experimental::IClockService::getComponentIID());
            FEP3_ARYA_LOGGER_LOG_ERROR(logger, error_desc.getDescription());
            return {error_desc, nullptr};
        }
    }
}

} // namespace fep3::native
