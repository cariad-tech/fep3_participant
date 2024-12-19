/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "clock_event_sink_registry.h"

#include <boost/asio/post.hpp>
#include <boost/bind.hpp>

#include <algorithm>
#include <memory>

namespace fep3 {
namespace native {

void ClockEventSinkRegistry::triggerEvent(const std::string& event_name,
                                          std::function<void(std::shared_ptr<IEventSink>)> func)
{
    _latch.reset(_event_sink_workers.size());

    {
        std::lock_guard<std::mutex> lock(_mtx);

        for (auto& worker: _event_sink_workers) {
            worker._worker->dispatch([&, event_name] {
                auto sink_ptr = worker._event_sink.getPtr();
                if (sink_ptr) {
                    func(sink_ptr);
                }
                else {
                    FEP3_LOG_DEBUG(
                        a_util::strings::format("Expired event sink addressed during '%s' "
                                                "event. Unregistering it from Event sink registry.",
                                                event_name.c_str()));
                    // TODO: Do we need ``unregisterSink``? If is null it will not be deregistered
                    // also erasing the list while iterating is dangerous
                    //
                    // unregisterSink(worker._event_sink);
                }

                _latch.count_down();
            });
        }
    }
    _latch.wait();
}

void ClockEventSinkRegistry::timeUpdateBegin(Timestamp old_time, Timestamp new_time)
{
    FEP3_LOG_DEBUG(a_util::strings::format("Distributing 'timeUpdateBegin' events. "
                                           "Old time '%lld', new time '%lld'.",
                                           old_time,
                                           new_time));

    triggerEvent("timeUpdateBegin", [&](std::shared_ptr<IEventSink> sink_ptr) {
        sink_ptr->timeUpdateBegin(old_time, new_time);
    });
}

void ClockEventSinkRegistry::timeUpdating(Timestamp new_time, std::optional<Timestamp> next_tick)
{
    FEP3_LOG_DEBUG(
        a_util::strings::format("Distributing 'timeUpdating' events. New time '%lld'.", new_time));

    triggerEvent("timeUpdating", [&](std::shared_ptr<IEventSink> sink_ptr) {
        sink_ptr->timeUpdating(new_time, next_tick);
    });
}

void ClockEventSinkRegistry::timeUpdateEnd(Timestamp new_time)
{
    FEP3_LOG_DEBUG(
        a_util::strings::format("Distributing 'timeUpdateEnd' events. New time '%lld'.", new_time));

    triggerEvent("timeUpdateEnd",
                 [&](std::shared_ptr<IEventSink> sink_ptr) { sink_ptr->timeUpdateEnd(new_time); });
}

void ClockEventSinkRegistry::timeResetBegin(Timestamp old_time, Timestamp new_time)
{
    FEP3_LOG_DEBUG(a_util::strings::format("Distributing 'timeResetBegin' events. Old "
                                           "time '%lld', new time '%lld'.",
                                           old_time,
                                           new_time));

    triggerEvent("timeResetBegin", [&](std::shared_ptr<IEventSink> sink_ptr) {
        sink_ptr->timeResetBegin(old_time, new_time);
    });
}

void ClockEventSinkRegistry::timeResetEnd(Timestamp new_time)
{
    FEP3_LOG_DEBUG(
        a_util::strings::format("Distributing 'timeResetEnd' events. New time '%lld'.", new_time));

    triggerEvent("timeResetEnd",
                 [&](std::shared_ptr<IEventSink> sink_ptr) { sink_ptr->timeResetEnd(new_time); });
}

} // namespace native
} // namespace fep3
