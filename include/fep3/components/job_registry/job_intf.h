/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

    This Source Code Form is subject to the terms of the Mozilla
    Public License, v. 2.0. If a copy of the MPL was not distributed
    with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

If it is not possible or desirable to put the notice in a particular file, then
You may include the notice in a location (such as a LICENSE file in a
relevant directory) where a recipient would be likely to look for such a notice.

You may add additional accurate notices of copyright ownership.

@endverbatim
 */


#pragma once

#include <memory>
#include <map>

#include <fep3/fep3_errors.h>
#include "job_info.h"
#include "fep3/fep3_timestamp.h"
#include <fep3/fep3_participant_export.h>

namespace fep3
{
namespace arya
{

/**
 * @brief Interface of a job
 */
class IJob
{
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

/// Entry of Jobs containing the @ref IJob and the @ref JobInfo
struct JobEntry
{
    /// The job itself
    std::shared_ptr<IJob> job;
    /// Info object about the job
    arya::JobInfo job_info;
};
/// map of job entries
using Jobs = std::map<std::string, arya::JobEntry>;

} // namespace arya
using arya::Jobs;
using arya::JobEntry;
using arya::IJob;
} // namespace fep3
