/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "scheduler_task.h"

namespace fep3::native {

void SchedulerTask::run(Timestamp wakeup_time)
{
    _task(wakeup_time);
}

bool SchedulerTask::operator<(const SchedulerTask& other) const
{
    return _next_instant < other._next_instant;
}

std::string SchedulerTask::getName() const
{
    return _name;
}

void SchedulerTask::setNextInstant(Timestamp next_instant)
{
    _next_instant = next_instant;
}

Timestamp SchedulerTask::getNextInstant() const
{
    return _next_instant;
}

Duration SchedulerTask::getPeriod() const
{
    return _period;
}

Duration SchedulerTask::getInitialDelay() const
{
    return _delay;
}

} // namespace fep3::native
