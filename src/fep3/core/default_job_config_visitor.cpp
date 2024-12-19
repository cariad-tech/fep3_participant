
/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "default_job_config_visitor.h"

namespace fep3 {
namespace core {

DefaultJobConfigVisitor::DefaultJobConfigVisitor(const std::string& name,
                                                 const IStreamType& stream_type,
                                                 std::list<core::DataReader>& readers,
                                                 size_t queue_size,
                                                 size_t purged_sample_log_capacity)
    : _name(name),
      _stream_type(stream_type),
      _readers(readers),
      _queue_size(queue_size),
      _purged_sample_log_capacity(purged_sample_log_capacity)
{
}

fep3::Result DefaultJobConfigVisitor::visitClockTriggeredConfiguration(
    const ClockTriggeredJobConfiguration&)
{
    _readers.emplace_back(
        _name, _stream_type, _queue_size, std::less{}, _purged_sample_log_capacity);

    return {};
}
fep3::Result DefaultJobConfigVisitor::visitDataTriggeredConfiguration(
    const DataTriggeredJobConfiguration&)
{
    _readers.emplace_back(
        _name, _stream_type, _queue_size, std::less_equal{}, _purged_sample_log_capacity);

    return {};
}

} // namespace core
} // namespace fep3
