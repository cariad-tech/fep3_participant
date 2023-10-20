/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2022 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include "datajob_base_job_configuration_visitor.h"

namespace fep3 {
namespace cpp {
DatajobBaseJobConfigurationVisitor::DatajobBaseJobConfigurationVisitor(
    const std::string& name,
    const IStreamType& stream_type,
    std::list<core::DataReader>& readers,
    const Optional<size_t>& queue_size)
    : _name(name), _stream_type(stream_type), _readers(readers), _queue_size(queue_size)
{
}

Result DatajobBaseJobConfigurationVisitor::visitClockTriggeredConfiguration(
    const ClockTriggeredJobConfiguration& /*configuration*/)
{
    if (_queue_size.has_value()) {
        _readers.emplace_back(_name, _stream_type, _queue_size.value());
    }
    else {
        _readers.emplace_back(_name, _stream_type);
    }

    return {};
}

Result DatajobBaseJobConfigurationVisitor::visitDataTriggeredConfiguration(
    const DataTriggeredJobConfiguration& /*configuration*/)
{
    if (_queue_size.has_value()) {
        _readers.emplace_back(_name, _stream_type, _queue_size.value(), std::less_equal{});
    }
    else {
        _readers.emplace_back(_name, _stream_type, std::less_equal{});
    }

    return {};
}

} // namespace cpp
} // namespace fep3
