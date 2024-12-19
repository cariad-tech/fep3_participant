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
namespace cpp {

class DatajobBaseJobConfigurationVisitor : public fep3::catelyn::IJobConfigurationVisitor {
public:
    DatajobBaseJobConfigurationVisitor(const std::string& name,
                                       const IStreamType& stream_type,
                                       std::list<core::DataReader>& readers,
                                       const Optional<size_t>& queue_size = Optional<size_t>());

public:
    Result visitClockTriggeredConfiguration(
        const ClockTriggeredJobConfiguration& configuration) override;
    Result visitDataTriggeredConfiguration(
        const DataTriggeredJobConfiguration& configuration) override;

private:
    const std::string _name;
    const IStreamType& _stream_type;
    std::list<core::DataReader>& _readers;
    const Optional<size_t> _queue_size;
};

} // namespace cpp
} // namespace fep3
