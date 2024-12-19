/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/cpp/datajob.h>

namespace fep3 {
namespace cpp {

fep3::Result DataJob::process(Timestamp /*time_of_execution*/)
{
    return {};
}

fep3::Result DataJob::reset()
{
    return Job::reset();
}

fep3::Result DataJob::executeDataIn(fep3::Timestamp time_of_execution)
{
    return dataIn(time_of_execution);
}

fep3::Result DataJob::executeDataOut(fep3::Timestamp time_of_execution)
{
    return dataOut(time_of_execution);
}

fep3::Result DataJob::addDataToComponents(const fep3::IComponents& components)
{
    return DataJobBase::addDataToComponents(components, getJobInfo().getName());
}

fep3::Result DataJob::removeDataFromComponents(const fep3::IComponents& components)
{
    return DataJobBase::removeDataFromComponents(components, getJobInfo().getName());
}

DataReader* DataJob::addDataIn(const std::string& name, const fep3::IStreamType& type)
{
    return DataJobBase::addDataIn(name, type, getJobInfo().getConfigCopy());
}

DataReader* DataJob::addDataIn(const std::string& name,
                               const fep3::IStreamType& type,
                               size_t queue_size)
{
    return DataJobBase::addDataIn(name, type, queue_size, getJobInfo().getConfigCopy());
}

} // namespace cpp
} // namespace fep3
