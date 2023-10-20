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

#include "fep3/fep3_duration.h"
#include "fep3/fep3_timestamp.h"

#include <functional>

namespace fep3::native {
class SchedulerTask {
public:
    template <typename T>
    SchedulerTask(
        T t, const std::string& name, Timestamp next_instant, Duration period, Duration delay)
        : _next_instant(next_instant),
          _period(period),
          _task([task = std::move(t)](Timestamp wakeup_time) { task(wakeup_time); }),
          _name(name),
          _delay(delay)
    {
    }

    void run(Timestamp wakeup_time);
    bool operator<(const SchedulerTask& other) const;
    std::string getName() const;
    void setNextInstant(Timestamp next_instant);
    Timestamp getNextInstant() const;
    Duration getPeriod() const;
    Duration getInitialDelay() const;

    // add some proper set get

private:
    Timestamp _next_instant;
    Duration _period;
    std::function<void(fep3::Timestamp)> _task;
    std::string _name;
    Duration _delay;
};
} // namespace fep3::native
