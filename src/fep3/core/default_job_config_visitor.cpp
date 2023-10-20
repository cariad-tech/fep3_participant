
/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2023 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include "default_job_config_visitor.h"

namespace fep3 {
namespace core {

DefaultJobConfigVisitor::DefaultJobConfigVisitor(const std::string& name,
                                                 const IStreamType& stream_type,
                                                 std::list<core::DataReader>& readers,
                                                 size_t queue_size)
    : _name(name), _stream_type(stream_type), _readers(readers), _queue_size(queue_size)
{
}

fep3::Result DefaultJobConfigVisitor::visitClockTriggeredConfiguration(
    const ClockTriggeredJobConfiguration&)
{
    _readers.emplace_back(_name, _stream_type);
    return {};
}
fep3::Result DefaultJobConfigVisitor::visitDataTriggeredConfiguration(
    const DataTriggeredJobConfiguration&)
{
    _readers.emplace_back(_name, _stream_type, std::less_equal{});
    return {};
}

} // namespace core
} // namespace fep3
