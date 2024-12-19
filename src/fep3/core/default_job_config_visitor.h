/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/job_registry/job_configuration.h>
#include <fep3/core/data/data_reader.h>

namespace fep3 {
namespace core {

class DefaultJobConfigVisitor : public fep3::catelyn::IJobConfigurationVisitor {
public:
    DefaultJobConfigVisitor(const std::string& name,
                            const IStreamType& stream_type,
                            std::list<core::DataReader>& readers,
                            size_t queue_size,
                            size_t purged_sample_log_capacity);

public:
    fep3::Result visitClockTriggeredConfiguration(const ClockTriggeredJobConfiguration&) override;
    fep3::Result visitDataTriggeredConfiguration(const DataTriggeredJobConfiguration&) override;

private:
    const std::string _name;
    const IStreamType& _stream_type;
    std::list<core::DataReader>& _readers;
    const size_t _queue_size;
    size_t _purged_sample_log_capacity;
};

} // namespace core
} // namespace fep3
