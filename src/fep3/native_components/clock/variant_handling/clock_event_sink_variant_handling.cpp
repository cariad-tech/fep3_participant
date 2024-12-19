/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

@endverbatim
 */
#include "clock_event_sink_variant_handling.h"

namespace fep3::native {

AryaToCatelynEventSinkAdapter::AryaToCatelynEventSinkAdapter(
    std::weak_ptr<fep3::arya::IClock::IEventSink> event_sink)
    : _event_sink(event_sink)
{
}

void AryaToCatelynEventSinkAdapter::timeUpdateBegin(fep3::arya::Timestamp old_time,
                                                    fep3::arya::Timestamp new_time)
{
    auto event_sink = _event_sink.lock();
    if (event_sink) {
        event_sink->timeUpdateBegin(old_time, new_time);
    }
}

void AryaToCatelynEventSinkAdapter::timeUpdating(fep3::arya::Timestamp new_time,
                                                 std::optional<fep3::arya::Timestamp>)
{
    auto event_sink = _event_sink.lock();
    if (event_sink) {
        event_sink->timeUpdating(new_time);
    }
}

void AryaToCatelynEventSinkAdapter::timeUpdateEnd(fep3::arya::Timestamp new_time)
{
    auto event_sink = _event_sink.lock();
    if (event_sink) {
        event_sink->timeUpdateEnd(new_time);
    }
}

void AryaToCatelynEventSinkAdapter::timeResetBegin(fep3::arya::Timestamp old_time,
                                                   fep3::arya::Timestamp new_time)
{
    auto event_sink = _event_sink.lock();
    if (event_sink) {
        event_sink->timeResetBegin(old_time, new_time);
    }
}

void AryaToCatelynEventSinkAdapter::timeResetEnd(fep3::arya::Timestamp new_time)
{
    auto event_sink = _event_sink.lock();
    if (event_sink) {
        event_sink->timeResetEnd(new_time);
    }
}

bool AryaToCatelynEventSinkAdapter::isEqual(
    std::weak_ptr<fep3::arya::IClock::IEventSink> event_sink) const
{
    return _event_sink.lock() == event_sink.lock();
}

CatelynToAryaEventSinkAdapter::CatelynToAryaEventSinkAdapter(
    std::weak_ptr<fep3::experimental::IClock::IEventSink> event_sink)
    : _event_sink(event_sink)
{
}

void CatelynToAryaEventSinkAdapter::setSink(
    std::weak_ptr<fep3::experimental::IClock::IEventSink> event_sink)
{
    _event_sink = event_sink;
}

void CatelynToAryaEventSinkAdapter::timeUpdateBegin(fep3::arya::Timestamp old_time,
                                                    fep3::arya::Timestamp new_time)
{
    auto event_sink = _event_sink.lock();
    if (event_sink) {
        event_sink->timeUpdateBegin(old_time, new_time);
    }
}

void CatelynToAryaEventSinkAdapter::timeUpdating(fep3::arya::Timestamp new_time)
{
    auto event_sink = _event_sink.lock();
    if (event_sink) {
        event_sink->timeUpdating(new_time, {});
    }
}

void CatelynToAryaEventSinkAdapter::timeUpdateEnd(fep3::arya::Timestamp new_time)
{
    auto event_sink = _event_sink.lock();
    if (event_sink) {
        event_sink->timeUpdateEnd(new_time);
    }
}

void CatelynToAryaEventSinkAdapter::timeResetBegin(fep3::arya::Timestamp old_time,
                                                   fep3::arya::Timestamp new_time)
{
    auto event_sink = _event_sink.lock();
    if (event_sink) {
        event_sink->timeResetBegin(old_time, new_time);
    }
}

void CatelynToAryaEventSinkAdapter::timeResetEnd(fep3::arya::Timestamp new_time)
{
    auto event_sink = _event_sink.lock();
    if (event_sink) {
        event_sink->timeResetEnd(new_time);
    }
}

bool CatelynToAryaEventSinkAdapter::isEqual(
    std::weak_ptr<fep3::experimental::IClock::IEventSink> event_sink) const
{
    return _event_sink.lock() == event_sink.lock();
}

std::shared_ptr<fep3::experimental::IClock::IEventSink> getCatelynEventSinkPointer(
    std::weak_ptr<fep3::arya::IClock::IEventSink> event_sink)
{
    auto ptr = event_sink.lock();
    if (ptr) {
        return std::make_shared<AryaToCatelynEventSinkAdapter>(ptr);
    }
    else
        return nullptr;
}

GenericEventSinkAdapter::GenericEventSinkAdapter(
    std::weak_ptr<fep3::experimental::IClock::IEventSink> event_sink)
    : _variant(event_sink)
{
}

GenericEventSinkAdapter::GenericEventSinkAdapter(
    std::weak_ptr<fep3::arya::IClock::IEventSink> event_sink)
    : _variant(event_sink)
{
}

std::shared_ptr<fep3::experimental::IClock::IEventSink> GenericEventSinkAdapter::getPtr() const
{
    return std::visit(
        [&](auto& arg) -> std::shared_ptr<fep3::experimental::IClock::IEventSink> {
            using SinkType = typename std::remove_reference_t<decltype(arg)>::element_type;
            if constexpr (std::is_same_v<SinkType, fep3::experimental::IClock::IEventSink>)
                return arg.lock();
            else
                return getCatelynEventSinkPointer(arg);
        },
        _variant);
}

} // namespace fep3::native
