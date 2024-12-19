/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/components/job_registry/job_info.h>
#include <fep3/fep3_timestamp.h>

#include <map>

namespace fep3 {
namespace arya {
/**
 * @brief Interface of a job
 */
class IJob {
protected:
    /// DTOR
    ~IJob() = default;

public:
    /**
     * @brief Read input samples.
     *
     * @param[in] time_of_execution Current simulation time
     * @return fep3::Result
     */
    virtual fep3::Result executeDataIn(arya::Timestamp time_of_execution) = 0;

    /**
     * @brief Process job.
     *
     * @param[in] time_of_execution Current simulation time
     * @return fep3::Result
     */
    virtual fep3::Result execute(arya::Timestamp time_of_execution) = 0;

    /**
     * @brief Write output samples.
     *
     * @param[in] time_of_execution Current simulation time
     * @return fep3::Result
     */
    virtual fep3::Result executeDataOut(arya::Timestamp time_of_execution) = 0;
};

/// Entry of Jobs containing the @ref arya::IJob and the @ref arya::JobInfo
struct JobEntry {
    /// The job itself
    std::shared_ptr<IJob> job;
    /// Info object about the job
    arya::JobInfo job_info;
};

/// map of arya::JobEntry
using Jobs = std::map<std::string, arya::JobEntry>;

} // namespace arya

namespace catelyn {

/// Entry of Jobs containing the @ref arya::IJob and the @ref catelyn::JobInfo
struct JobEntry {
    /// The job itself
    std::shared_ptr<arya::IJob> job;
    /// Info object about the job
    catelyn::JobInfo job_info;
};
///@cond nodoc
using Jobs = std::map<std::string, catelyn::JobEntry>;
///@endcond nodoc

} // namespace catelyn

using arya::IJob;
using catelyn::JobEntry;
using catelyn::Jobs;
} // namespace fep3
