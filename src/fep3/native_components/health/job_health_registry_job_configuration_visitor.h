/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "job_health_registry.h"

namespace fep3 {
namespace native {

class JobHealthRegistryJobConfigurationVisitor : public fep3::catelyn::IJobConfigurationVisitor {
public:
    JobHealthRegistryJobConfigurationVisitor(
        std::vector<IJobHealthRegistry::JobHealthiness>& jobs_healthiness,
        const catelyn::JobEntry* current_processed_job);

public:
    virtual Result visitClockTriggeredConfiguration(
        const ClockTriggeredJobConfiguration& configuration) override;
    virtual Result visitDataTriggeredConfiguration(
        const DataTriggeredJobConfiguration& configuration) override;

private:
    std::vector<IJobHealthRegistry::JobHealthiness>& _jobs_healthiness;
    const catelyn::JobEntry* _current_processed_job;
};

} // namespace native
} // namespace fep3
