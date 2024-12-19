/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "variant_handling/clock_event_sink_variant_handling.h"

#include <fep3/base/thread/single_thread_worker.h>
#include <fep3/components/clock/clock_service_intf.h>
#include <fep3/components/logging/easy_logger.h>

#include <boost/thread/latch.hpp>

#include <mutex>

namespace fep3 {
namespace native {

class ClockEventSinkRegistry : public experimental::IClock::IEventSink, public base::EasyLogging {
private:
    struct EventSinkWorker {
        EventSinkWorker(GenericEventSinkAdapter event_sink,
                        std::shared_ptr<fep3::base::SingleThreadWorker> worker)
            : _event_sink(event_sink), _worker(std::move(worker))
        {
        }

        GenericEventSinkAdapter _event_sink;
        std::shared_ptr<fep3::base::SingleThreadWorker> _worker;
    };

public:
    ClockEventSinkRegistry() : _latch(0)
    {
    }

    ~ClockEventSinkRegistry()
    {
    }

    template <typename T>
    fep3::Result registerSink(const std::weak_ptr<T>& sink)
    {
        const auto sink_ptr = sink.lock();
        if (sink_ptr) {
            std::lock_guard<std::mutex> lock_guard(_mtx);

            auto it = std::find_if(
                _event_sink_workers.begin(),
                _event_sink_workers.end(),
                [&sink_ptr](const auto& ele) { return ele._event_sink.isEqual(sink_ptr); });

            if (it != _event_sink_workers.end()) {
                FEP3_LOG_WARNING("Registration of event sink registry failed. Event sink exists.");
                return fep3::ERR_FAILED;
            }

            _event_sink_workers.emplace_back(GenericEventSinkAdapter(sink),
                                             std::make_shared<fep3::base::SingleThreadWorker>());

            FEP3_LOG_DEBUG("Registered event sink at the clock event sink registry.");
            return {};
        }
        else {
            FEP3_LOG_WARNING("Registration of invalid event sink at the clock event sink "
                             "registry failed.");
            return fep3::ERR_INVALID_ARG;
        }
    }

    // event sinks can only be registered and deregistered with the same namespace (ex. arya or
    // catelyn)
    template <typename T>
    fep3::Result unregisterSink(const std::weak_ptr<T>& sink)
    {
        const auto sink_ptr = sink.lock();
        if (sink_ptr) {
            std::lock_guard<std::mutex> lock_guard(_mtx);

            auto it = std::find_if(
                _event_sink_workers.begin(),
                _event_sink_workers.end(),
                [&sink_ptr](const auto& ele) { return ele._event_sink.isEqual(sink_ptr); });

            if (it == _event_sink_workers.end()) {
                FEP3_LOG_WARNING("Deregistration of event sink from the clock event sink "
                                 "registry failed. Event sink not found in the registry.");
                return fep3::ERR_FAILED;
            }

            _event_sink_workers.erase(it);
            FEP3_LOG_DEBUG("Unregistered event sink from the clock event sink "
                           "registry.");

            return {};
        }
        else {
            FEP3_LOG_WARNING("Deregistration of invalid event sink from the clock event "
                             "sink registry failed.");
            return fep3::ERR_INVALID_ARG;
        }
    }

private:
    void timeUpdateBegin(Timestamp old_time, Timestamp new_time) override;
    void timeUpdating(Timestamp new_time, std::optional<Timestamp> next_tick) override;
    void timeUpdateEnd(Timestamp new_time) override;
    void timeResetBegin(Timestamp old_time, Timestamp new_time) override;
    void timeResetEnd(Timestamp new_time) override;

    void triggerEvent(const std::string& event_name,
                      std::function<void(std::shared_ptr<IEventSink>)>);

private:
    std::mutex _mtx;
    std::list<EventSinkWorker> _event_sink_workers;
    boost::latch _latch;
};

} // namespace native
} // namespace fep3
