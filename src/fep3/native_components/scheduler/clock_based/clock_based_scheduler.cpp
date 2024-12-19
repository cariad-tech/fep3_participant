/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "clock_based_scheduler.h"

#include "scheduler_factory.h"

#include <fep3/components/logging/easy_logger.h>
#include <fep3/native_components/clock/variant_handling/clock_service_handling.h>

namespace fep3::native {

ClockBasedScheduler::ClockBasedScheduler(const std::shared_ptr<const fep3::ILogger> logger)
    : ClockBasedScheduler(logger, std::make_shared<SchedulerFactory>())
{
}

ClockBasedScheduler::ClockBasedScheduler(
    std::shared_ptr<const fep3::ILogger> logger,
    std::shared_ptr<const fep3::native::ISchedulerFactory> scheduler_factory)
    : _logger(logger), _scheduler_factory(std::move(scheduler_factory))
{
    if (!_logger) {
        throw std::runtime_error("Logger not set");
    }
}

ClockBasedScheduler::~ClockBasedScheduler()
{
}

std::string ClockBasedScheduler::getName() const
{
    return "clock_based_scheduler";
}

fep3::Result ClockBasedScheduler::initialize(fep3::arya::IClockService& clock,
                                             const fep3::arya::Jobs& jobs)
{
    auto job_count = jobs.size();
    if (job_count > 0) {
        _thread_pool = std::make_unique<ThreadPoolExecutor>(job_count);
    }

    _data_triggered_executor = std::make_unique<DataTriggeredExecutor>(*_thread_pool);

    FEP3_RETURN_IF_FAILED(initializeTimerScheduler(clock));

    for (auto& job: jobs) {
        addClockTriggeredJobToScheduler(
            job.second.job_info.getConfig(), *(job.second.job), job.second.job_info.getName());
    }

    return {};
}

template <typename T>
fep3::Result ClockBasedScheduler::initializeTimerScheduler(T& clock_service)
{
    auto [result, clock_service_adapter] = getClockServiceAdapter(clock_service, _logger);
    FEP3_RETURN_IF_FAILED(result);
    _clock_service = std::move(clock_service_adapter);

    _task_executor = std::make_shared<TaskClockEventSink>(_clock_service->getType(),
                                                          _clock_service->getTimeGetter(),
                                                          _logger,
                                                          _scheduler_factory,
                                                          *_thread_pool);

    FEP3_RETURN_IF_FAILED(_clock_service->registerEventSink(_task_executor));

    return {};
}

fep3::Result ClockBasedScheduler::initialize(const fep3::IComponents& components)
{
    const auto health_service = components.getComponent<IHealthService>();
    if (!health_service) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER,
                                 "access to component IHealthService was not possible, make sure "
                                 "component is set in components file");
    }

    _health_service = health_service;

    const auto data_registry = components.getComponent<IDataRegistry>();

    if (!data_registry) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER,
                                 "access to component IDataRegistry was not possible, make sure "
                                 "component is set in components file");
    }

    _data_registry = data_registry;

    const auto job_registry = components.getComponent<fep3::IJobRegistry>();
    if (!job_registry) {
        RETURN_ERROR_DESCRIPTION(ERR_POINTER, "access to component IJobRegistry was not possible");
    }

    const auto jobs = job_registry->getJobsCatelyn();

    auto job_count = jobs.size();
    if (job_count > 0) {
        _thread_pool = std::make_unique<ThreadPoolExecutor>(job_count);
        FEP3_ARYA_LOGGER_LOG_DEBUG(
            _logger,
            a_util::strings::format("Thread pool of scheduler initialized with %llu threads",
                                    job_count));
    }
    else {
        FEP3_ARYA_LOGGER_LOG_DEBUG(_logger,
                                   "No jobs found, thread pool of scheduler will have 0 threads");
    }

    _data_triggered_executor = std::make_unique<DataTriggeredExecutor>(*_thread_pool);

    initializeTimerScheduler(components);

    for (auto& job: jobs) {
        FEP3_RETURN_IF_FAILED(parseJobEntry(job.second));
    }

    return {};
}

fep3::Result ClockBasedScheduler::visitDataTriggeredConfiguration(
    const fep3::catelyn::DataTriggeredJobConfiguration& config)
{
    fep3::native::JobRunner job_runner(_current_processed_job->job_info.getName(),
                                       config._runtime_violation_strategy,
                                       config._max_runtime_real_time,
                                       _logger,
                                       *_health_service);

    for (auto& signal_name: config._signal_names) {
        auto data_triggered_receiver =
            std::make_shared<DataTriggeredReceiver<>>(_clock_service->getTimeGetter(),
                                                      _current_processed_job->job,
                                                      signal_name,
                                                      job_runner,
                                                      *_data_triggered_executor,
                                                      _logger);
        auto res =
            _data_registry->registerDataReceiveListener(signal_name, data_triggered_receiver);

        if (res) {
            FEP3_ARYA_LOGGER_LOG_DEBUG(
                _logger,
                a_util::strings::format(
                    "Adding listener for signal %s in data triggered job '%s' succeeded",
                    signal_name.c_str(),
                    _current_processed_job->job_info.getName().c_str()));
        }
        else {
            FEP3_ARYA_LOGGER_LOG_WARNING(
                _logger,
                a_util::strings::format(
                    "Adding listener for signal %s in data triggered job '%s' failed, error :%s",
                    signal_name.c_str(),
                    _current_processed_job->job_info.getName().c_str(),
                    res.getDescription()));
        }

        FEP3_RETURN_IF_FAILED(res);
        _data_triggered_receivers.push_back(data_triggered_receiver);
    }

    return {};
}

fep3::Result ClockBasedScheduler::visitClockTriggeredConfiguration(
    const fep3::catelyn::ClockTriggeredJobConfiguration& config)
{
    addClockTriggeredJobToScheduler(config,
                                    *(_current_processed_job->job),
                                    _current_processed_job->job_info.getName(),
                                    *_health_service);

    return {};
}

fep3::Result ClockBasedScheduler::parseJobEntry(const catelyn::JobEntry& job_entry)
{
    _current_processed_job = &job_entry;
    auto job_config = _current_processed_job->job_info.getConfigCopy();
    return job_config->acceptVisitor(*this);
}

template <typename Config, typename... Args>
void ClockBasedScheduler::addClockTriggeredJobToScheduler(const Config& config,
                                                          fep3::IJob& i_job,
                                                          const std::string& job_name,
                                                          Args&&... args)
{
    fep3::native::JobRunner job_runner(job_name,
                                       config._runtime_violation_strategy,
                                       config._max_runtime_real_time,
                                       _logger,
                                       std::forward<Args>(args)...);
    auto res = _task_executor->addTask(
        [&, job_runner, this](const fep3::Timestamp time) mutable {
            job_runner.runJob(time, i_job);
        },
        job_name,
        config._cycle_sim_time,
        config._delay_sim_time);

    if (!res) {
        FEP3_ARYA_LOGGER_LOG_WARNING(
            _logger,
            a_util::strings::format("Adding clock triggered job '%s' failed, error :%s",
                                    job_name.c_str(),
                                    res.getDescription()));
    }
    else {
        FEP3_ARYA_LOGGER_LOG_DEBUG(
            _logger,
            a_util::strings::format("Adding clock triggered job '%s' succeeded", job_name.c_str()));
    }
}

fep3::Result ClockBasedScheduler::start()
{
    if (_thread_pool) {
        _thread_pool->start();
    }

    if (_data_triggered_executor) {
        _data_triggered_executor->start();
    }

    if (_task_executor) {
        FEP3_RETURN_IF_FAILED(_task_executor->start());
    }

    FEP3_ARYA_LOGGER_LOG_DEBUG(_logger, "Started scheduler");
    return {};
}

fep3::Result ClockBasedScheduler::stop()
{
    if (_thread_pool) {
        _thread_pool->stop();
    }

    if (_data_triggered_executor) {
        _data_triggered_executor->stop();
    }

    if (_task_executor) {
        FEP3_RETURN_IF_FAILED(_task_executor->stop());
    }

    return {};
}

fep3::Result ClockBasedScheduler::deinitialize()
{
    stop();

    if (_thread_pool) {
        _thread_pool.reset();
    }

    if (_clock_service) {
        _clock_service->unregisterEventSink(_task_executor);
    }

    _task_executor.reset();
    _clock_service.reset();

    if (_data_registry) {
        for (auto& data_trigg_receiver: _data_triggered_receivers) {
            _data_registry->unregisterDataReceiveListener(data_trigg_receiver->getSignalName(),
                                                          data_trigg_receiver);
        }

        _data_triggered_receivers.clear();
        _data_registry = nullptr;
    }

    _data_triggered_executor.reset();

    return {};
}

} // namespace fep3::native
