/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include "scenario_helpers.h"

#include <fep3/components/clock/mock_clock.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/components/service_bus/rpc/fep_rpc_stubs_client.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/core/participant_executor.hpp>
#include <fep3/rpc_services/logging/logging_client_stub.h>
#include <fep3/rpc_services/logging/logging_service_rpc_intf_def.h>

#include <gtest_asserts.h>

namespace fep3 {
namespace test {
namespace scenario {

class ParticipantWrapper {
public:
    ParticipantWrapper(const std::shared_ptr<fep3::base::Participant>& participant)
        : participant(participant)
    {
        setupLoggingSink();
    }

    fep3::IJob* getJob(const std::string& name)
    {
        const auto job_registry = participant->getComponent<fep3::IJobRegistry>();
        const auto jobs = job_registry->getJobs();

        if (0 == jobs.count(name)) {
            return nullptr;
        }
        auto& job_entry = jobs.at(name);
        return job_entry.job.get();
    }

private:
    void setupLoggingSink()
    {
        logging_sink =
            std::make_shared<::testing::NiceMock<fep3::mock::LoggingService::LoggingSink>>();

        auto logging_service = participant->getComponent<fep3::ILoggingService>();
        ASSERT_FEP3_NOERROR(logging_service->registerSink("mock_sink", logging_sink));

        EXPECT_CALL(*logging_sink, log(::testing::_)).WillRepeatedly(testing::Return(Result{}));
        EXPECT_CALL(*logging_sink, log(logIsError())).Times(0);

        setLoggerFilter();
    }

    void setLoggerFilter()
    {
        using LoggingServiceClient =
            fep3::rpc::RPCServiceClient<fep3::rpc_stubs::RPCLoggingClientStub,
                                        fep3::rpc::IRPCLoggingServiceDef>;

        const auto participant_name = participant->getName();
        auto service_bus = participant->getComponent<fep3::IServiceBus>();
        auto logging_client =
            LoggingServiceClient(fep3::rpc::IRPCLoggingServiceDef::getRPCDefaultName(),
                                 service_bus->getRequester(participant_name));
        ASSERT_EQ(logging_client.setLoggerFilter(
                      "mock_sink,console", "", static_cast<int>(fep3::LoggerSeverity::debug)),
                  0);
    }

public:
    std::shared_ptr<fep3::base::Participant> participant;

public:
    std::shared_ptr<fep3::mock::LoggingService::LoggingSink> logging_sink;
};

class ParticipantStateMachine : public IStateMachine {
public:
    ParticipantStateMachine(std::shared_ptr<fep3::core::ParticipantExecutor> executor)
        : _executor(executor)
    {
    }

    void Running() override
    {
        if (ParticipantState::UNLOADED == _current_state) {
            ASSERT_TRUE(_executor->load());
            _current_state = ParticipantState::LOADED;
            ASSERT_TRUE(_executor->initialize());
            _current_state = ParticipantState::INITIALIZED;
            ASSERT_TRUE(_executor->start());
            _current_state = ParticipantState::RUNNING;
        }
        else if (ParticipantState::INITIALIZED == _current_state) {
            ASSERT_TRUE(_executor->start());
            _current_state = ParticipantState::RUNNING;
        }
        else {
            throw std::runtime_error(
                a_util::strings::format("Transition from '%s' to '%s' not supported",
                                        getStateName(_current_state).value().c_str(),
                                        getStateName(ParticipantState::RUNNING).value().c_str()));
        }
    }

    void Initialized() override
    {
        if (ParticipantState::RUNNING == _current_state) {
            ASSERT_TRUE(_executor->stop());
            _current_state = ParticipantState::INITIALIZED;
            ASSERT_TRUE(_executor->deinitialize());
            _current_state = ParticipantState::LOADED;
            ASSERT_TRUE(_executor->unload());
            _current_state = ParticipantState::UNLOADED;
        }
        else if (ParticipantState::UNLOADED == _current_state) {
            ASSERT_TRUE(_executor->load());
            _current_state = ParticipantState::LOADED;
            ASSERT_TRUE(_executor->initialize());
            _current_state = ParticipantState::INITIALIZED;
        }
        else {
            throw std::runtime_error(a_util::strings::format(
                "Transition from '%s' to '%s' not supported",
                getStateName(_current_state).value().c_str(),
                getStateName(ParticipantState::INITIALIZED).value().c_str()));
        }
    }

private:
    ParticipantState _current_state = ParticipantState::UNLOADED;
    std::shared_ptr<fep3::core::ParticipantExecutor> _executor;
};

} // namespace scenario
} // namespace test
} // namespace fep3
