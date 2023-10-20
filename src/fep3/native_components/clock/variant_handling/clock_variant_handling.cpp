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

#include "clock_variant_handling.h"

#include "clock_event_sink_variant_handling.h"

#include <fep3/components/logging/easy_logger.h>

namespace fep3::native {

ClockAdapterAryaToCatelyn::ClockAdapterAryaToCatelyn(std::shared_ptr<fep3::arya::IClock> clock)
    : _clock(clock)
{
}

void ClockAdapterAryaToCatelyn::start(
    const std::weak_ptr<fep3::experimental::IClock::IEventSink>& event_sink)
{
    _event_sink_adapter = std::make_shared<CatelynToAryaEventSinkAdapter>(event_sink);
    _clock->start(_event_sink_adapter);
}

void ClockAdapterAryaToCatelyn::stop()
{
    _clock->stop();
}

std::string ClockAdapterAryaToCatelyn::getName() const
{
    return _clock->getName();
}

fep3::arya::IClock::ClockType ClockAdapterAryaToCatelyn::getType() const
{
    return _clock->getType();
}
fep3::arya::Timestamp ClockAdapterAryaToCatelyn::getTime() const
{
    return _clock->getTime();
}
void ClockAdapterAryaToCatelyn::reset(fep3::arya::Timestamp new_time)
{
    _clock->reset(new_time);
}

ClockAdapterCatelynToArya::ClockAdapterCatelynToArya(
    std::shared_ptr<fep3::experimental::IClock> clock)
    : _clock(clock)
{
}

void ClockAdapterCatelynToArya::start(
    const std::weak_ptr<fep3::arya::IClock::IEventSink>& event_sink)
{
    _event_sink_adapter = std::make_shared<AryaToCatelynEventSinkAdapter>(event_sink);
    _clock->start(_event_sink_adapter);
}

void ClockAdapterCatelynToArya::stop()
{
    _clock->stop();
}

std::string ClockAdapterCatelynToArya::getName() const
{
    return _clock->getName();
}

fep3::arya::IClock::ClockType ClockAdapterCatelynToArya::getType() const
{
    return _clock->getType();
}
fep3::arya::Timestamp ClockAdapterCatelynToArya::getTime() const
{
    return _clock->getTime();
}
void ClockAdapterCatelynToArya::reset(fep3::arya::Timestamp new_time)
{
    _clock->reset(new_time);
}

GenericClockAdapter::~GenericClockAdapter()
{
}
std::string GenericClockAdapter::getName() const
{
    return std::visit([](auto& clock) -> std::string { return clock->getName(); }, _clock);
}

void GenericClockAdapter::start(
    const std::weak_ptr<fep3::experimental::IClock::IEventSink>& event_sink)
{
    std::visit(
        [&](auto& arg) {
            using ClockTypeInVariant =
                typename std::remove_reference_t<decltype(arg)>::element_type;
            if constexpr (std::is_same_v<ClockTypeInVariant, fep3::experimental::IClock>)
                arg->start(event_sink);
            else {
                _adapter = std::make_shared<CatelynToAryaEventSinkAdapter>(event_sink);
                arg->start(_adapter);
            }
        },
        _clock);
}

void GenericClockAdapter::stop()
{
    std::visit([&](auto& clock) { clock->stop(); }, _clock);
}

fep3::arya::IClock::ClockType GenericClockAdapter::getType() const
{
    return std::visit([](auto& clock) -> fep3::arya::IClock::ClockType { return clock->getType(); },
                      _clock);
}

arya::Timestamp GenericClockAdapter::getTime() const
{
    return std::visit([](auto& clock) -> arya::Timestamp { return clock->getTime(); }, _clock);
}

void GenericClockAdapter::reset(fep3::arya::Timestamp new_time)
{
    std::visit([&](auto& clock) { clock->reset(new_time); }, _clock);
}

std::shared_ptr<fep3::experimental::IClock> GenericClockAdapter::getClockAdapter(
    std::shared_ptr<fep3::arya::IClock> clock) const
{
    return std::make_shared<ClockAdapterAryaToCatelyn>(clock);
}

std::shared_ptr<fep3::arya::IClock> GenericClockAdapter::getClockAdapter(
    std::shared_ptr<fep3::experimental::IClock> clock) const
{
    return std::make_shared<ClockAdapterCatelynToArya>(clock);
}

fep3::Result unregisterClockFromService(const std::string& clock_name,
                                        const IComponents& components,
                                        std::shared_ptr<ILogger> logger)
{
    auto clock_service_catelyn = components.getComponent<fep3::experimental::IClockService>();

    if (!clock_service_catelyn) {
        auto clock_service_arya = components.getComponent<fep3::arya::IClockService>();
        // no clock service available
        if (!clock_service_arya) {
            RETURN_ERROR_DESCRIPTION(ERR_UNEXPECTED, "Arya Clock Service is not registered");
        }
        // only arya clock service available
        else {
            FEP3_ARYA_LOGGER_LOG_WARNING(logger,
                                         "Catelyn clock service not found, it is recommended to "
                                         "add catelyn clock service to the loaded components")
            return clock_service_arya->unregisterClock(clock_name);
        }
    }
    else {
        return clock_service_catelyn->unregisterClock(clock_name);
    }
}

} // namespace fep3::native
