/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "data_triggered_receiver.h"
#include "task_clock_event_sink.h"
#include "threaded_executor.h"

#include <fep3/components/data_registry/data_registry_intf.h>
#include <fep3/components/logging/logger_intf.h>
#include <fep3/native_components/clock/variant_handling/clock_variant_handling.h>

namespace fep3::native {

struct ISchedulerFactory;
class ClockServiceAdapter;

class ClockBasedScheduler : public fep3::catelyn::IScheduler,
                            public fep3::catelyn::IJobConfigurationVisitor {
public:
    ClockBasedScheduler(std::shared_ptr<const fep3::ILogger> logger,
                        std::shared_ptr<const fep3::native::ISchedulerFactory> scheduler_factory);
    ClockBasedScheduler(std::shared_ptr<const fep3::ILogger> logger);
    ~ClockBasedScheduler();

public:
    std::string getName() const override;

    fep3::Result initialize(fep3::arya::IClockService& clock,
                            const fep3::arya::Jobs& jobs) override;
    fep3::Result initialize(const fep3::IComponents& components) override;
    fep3::Result start() override;
    fep3::Result stop() override;
    fep3::Result deinitialize() override;

public:
    fep3::Result visitClockTriggeredConfiguration(
        const fep3::catelyn::ClockTriggeredJobConfiguration&) override;
    fep3::Result visitDataTriggeredConfiguration(
        const fep3::catelyn::DataTriggeredJobConfiguration&) override;

private:
    fep3::Result parseJobEntry(const catelyn::JobEntry& job_entry);

    template <typename Config, typename... Args>
    void addClockTriggeredJobToScheduler(const Config& config,
                                         fep3::IJob& i_job,
                                         const std::string& job_name,
                                         Args&&... args);

    template <typename T>
    fep3::Result initializeTimerScheduler(T& clock_service);

private:
    std::unique_ptr<IThreadPoolExecutor> _thread_pool;
    std::shared_ptr<TaskClockEventSink> _task_executor;
    std::shared_ptr<const fep3::ILogger> _logger;
    std::unique_ptr<ClockServiceAdapter> _clock_service;
    fep3::IDataRegistry* _data_registry = nullptr;
    fep3::IHealthService* _health_service = nullptr;
    const catelyn::JobEntry* _current_processed_job = nullptr;
    std::unique_ptr<fep3::native::DataTriggeredExecutor> _data_triggered_executor;
    std::vector<std::shared_ptr<DataTriggeredReceiver<>>> _data_triggered_receivers;
    std::shared_ptr<const fep3::native::ISchedulerFactory> _scheduler_factory;
    std::shared_ptr<CatelynToAryaEventSinkAdapter> _adapter;
};

} // namespace fep3::native
