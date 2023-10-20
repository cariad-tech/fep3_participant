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

#include "../../src/fep3/core/mock/mock_core.h"
#include "../helper/platform_dep_name.h"
#include "scenario_system.h"

#include <fep3/components/clock_sync/clock_sync_service_intf.h>
#include <fep3/cpp/participant.h>

namespace fep3 {
namespace test {
namespace scenario {

struct NParticipantSystem : public ::testing::Test, public IStateMachine {
    void SetUp(const std::vector<std::shared_ptr<fep3::base::Participant>>& participants,
               const std::string& master_name)
    {
        for (auto& participant: participants) {
            _executors.push_back(std::make_shared<fep3::core::ParticipantExecutor>(*participant));
            _executors.back()->exec();
            _participant_wrappers.push_back(std::make_shared<ParticipantWrapper>(participant));
        }

        _master_name = master_name;
        _system = std::make_unique<SystemStateMachine>(_executors);
    }

    std::string getTimingMasterName() const
    {
        return _master_name;
    }

    std::vector<std::string> getParticipantNames() const
    {
        std::vector<std::string> names;
        std::transform(_participant_wrappers.begin(),
                       _participant_wrappers.end(),
                       std::back_inserter(names),
                       [this](std::shared_ptr<ParticipantWrapper> wrapper) {
                           return wrapper->participant->getName();
                       });
        return names;
    }

    std::vector<std::shared_ptr<fep3::base::Participant>> getTimingSlaves() const
    {
        std::vector<std::shared_ptr<fep3::base::Participant>> slaves;

        auto participants = getParticipants();
        std::copy_if(participants.begin(),
                     participants.end(),
                     std::back_inserter(slaves),
                     [this](std::shared_ptr<fep3::base::Participant> participant) {
                         return participant->getName() != _master_name;
                     });
        return slaves;
    }

    fep3::base::Participant* getParticipant(const std::string& name) const
    {
        auto participants = getParticipants();
        auto result = std::find_if(participants.begin(),
                                   participants.end(),
                                   [name](std::shared_ptr<fep3::base::Participant> participant) {
                                       return participant->getName() == name;
                                   });

        if (result == participants.end()) {
            return nullptr;
        }

        return result->get();
    }

    ParticipantWrapper* getWrapper(const std::string& name) const
    {
        auto result = std::find_if(_participant_wrappers.begin(),
                                   _participant_wrappers.end(),
                                   [name](std::shared_ptr<ParticipantWrapper> wrapper) {
                                       return wrapper->participant->getName() == name;
                                   });

        if (result == _participant_wrappers.end()) {
            return nullptr;
        }

        return result->get();
    }

    std::vector<std::shared_ptr<fep3::base::Participant>> getParticipants() const
    {
        std::vector<std::shared_ptr<fep3::base::Participant>> participants;
        std::transform(
            _participant_wrappers.begin(),
            _participant_wrappers.end(),
            std::back_inserter(participants),
            [this](std::shared_ptr<ParticipantWrapper> wrapper) { return wrapper->participant; });
        return participants;
    }

    fep3::Result configureTimingMaster(
        const std::vector<std::pair<std::string, std::string>>& pairs_of_properties)
    {
        auto master = getParticipant(getTimingMasterName());
        if (!master) {
            return fep3::ERR_NOT_FOUND;
        }
        return configureParticipant(pairs_of_properties, *master);
    }

    fep3::Result configureTimingSlaves(
        const std::vector<std::pair<std::string, std::string>>& pairs_of_properties)
    {
        return configureParticipants(pairs_of_properties, getTimingSlaves());
    }

    fep3::Result configureAllParticipants(
        const std::vector<std::pair<std::string, std::string>>& pairs_of_properties)
    {
        return configureParticipants(pairs_of_properties, getParticipants());
    }

    void Running() override
    {
        _system->Running();
    }

    void Initialized() override
    {
        std::vector<std::shared_ptr<fep3::base::Participant>> participants = getParticipants();
        std::vector<std::string> part_names = getParticipantNames();

        for (auto& participant: participants) {
            const uint8_t try_count = 5;
            bool discovered_all = false;
            for (uint8_t i = 0; i < try_count; ++i) {
                auto service_bus = participant->getComponent<fep3::IServiceBus>();
                auto sys_access = service_bus->getSystemAccess();
                auto discovered_parts = sys_access->discover(std::chrono::milliseconds(4000));
                if (std::all_of(part_names.begin(), part_names.end(), [&](const std::string& s) {
                        return discovered_parts.count(s) != 0;
                    })) {
                    discovered_all = true;
                    break;
                }
            }
            if (!discovered_all) {
                throw std::runtime_error("Test will fail, not all participants are discovered");
            }
        }

        _system->Initialized();
    }

public:
    std::string _system_name = makePlatformDepName("test_system");
    std::string _system_version = "test_version";

private:
    std::string _master_name;
    std::vector<std::shared_ptr<ParticipantWrapper>> _participant_wrappers;
    std::vector<std::shared_ptr<fep3::core::ParticipantExecutor>> _executors;
    std::unique_ptr<SystemStateMachine> _system;
};

template <typename job_type>
struct MyElement : fep3::core::ElementBase {
    MyElement()
        : fep3::core::ElementBase("my_element", "my_version"), _job(std::make_shared<job_type>())
    {
    }

    fep3::Result initialize() override
    {
        if (!_job) {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ADDRESS,
                                     "Job was not initialized in element.");
        }
        const auto components = getComponents();
        if (!components) {
            RETURN_ERROR_DESCRIPTION(fep3::ERR_INVALID_ADDRESS, "Components inaccessible.");
        }

        FEP3_RETURN_IF_FAILED(fep3::core::addToComponents({_job}, *components));

        return {};
    }

private:
    std::shared_ptr<job_type> _job;
};

struct MyCoreJob_100ms : ::testing::NiceMock<fep3::mock::core::Job> {
    MyCoreJob_100ms()
        : ::testing::NiceMock<fep3::mock::core::Job>("core_job_100ms",
                                                     fep3::Duration{std::chrono::milliseconds(100)})
    {
    }
};

struct MasterSlaveSystem : public NParticipantSystem {
    void SetUp() override
    {
        NParticipantSystem::SetUp(createParticipants(), getMasterName());
    }

    /**
     * Override to provide your own participants
     */
    virtual std::vector<std::shared_ptr<fep3::base::Participant>> createParticipants() const
    {
        const std::string master_name{"test_timing_master"};
        const std::string slave_name{"test_timing_slave"};

        auto master = std::make_shared<fep3::base::Participant>(
            fep3::cpp::createParticipant<MyElement<MyCoreJob_100ms>>(master_name, _system_name));

        auto slave = std::make_shared<fep3::base::Participant>(
            fep3::cpp::createParticipant<MyElement<MyCoreJob_100ms>>(slave_name, _system_name));

        return {master, slave};
    }

    /**
     * Override to provide timing master name
     */
    std::string getMasterName() const
    {
        return "test_timing_master";
    }
};

struct MasterSlaveSystemDiscrete : public MasterSlaveSystem {
    void SetUp() override
    {
        MasterSlaveSystem::SetUp();

        ASSERT_FEP3_NOERROR(configureTimingMaster(
            {std::make_pair(FEP3_CLOCK_SERVICE_MAIN_CLOCK, FEP3_CLOCK_LOCAL_SYSTEM_SIM_TIME)}));

        ASSERT_FEP3_NOERROR(configureTimingSlaves(
            {std::make_pair(FEP3_CLOCK_SERVICE_MAIN_CLOCK,
                            FEP3_CLOCK_SLAVE_MASTER_ONDEMAND_DISCRETE),
             std::make_pair(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER, getMasterName())}));
    }
    static const int _number_of_time_resets = 1;
};

struct MasterSlaveSystemContinuous : public MasterSlaveSystem {
    void SetUp() override
    {
        MasterSlaveSystem::SetUp();

        ASSERT_FEP3_NOERROR(configureTimingMaster(
            {std::make_pair(FEP3_CLOCK_SERVICE_MAIN_CLOCK, FEP3_CLOCK_LOCAL_SYSTEM_REAL_TIME)}));

        ASSERT_FEP3_NOERROR(configureTimingSlaves(
            {std::make_pair(FEP3_CLOCK_SERVICE_MAIN_CLOCK, FEP3_CLOCK_SLAVE_MASTER_ONDEMAND),
             std::make_pair(FEP3_CLOCKSYNC_SERVICE_CONFIG_TIMING_MASTER, getMasterName())}));
    }
    static const int _number_of_time_resets = 2;
};

} // namespace scenario
} // namespace test
} // namespace fep3