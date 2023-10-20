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

#include <fep3/components/job_registry/job_configuration.h>
#include <fep3/fep3_errors.h>

namespace fep3 {
namespace arya {
/**
 * @brief JobInfo contains the name of a job and the corresponding job configuration.
 *
 * Usually JobInfos are used to retrieve information regarding jobs registered at the job registry.
 */
class JobInfo final {
public:
    /**
     * @brief CTOR
     *
     * @param[in] name Name of the job
     * @param[in] cycle_time Cycle time of the job
     */
    inline JobInfo(const std::string& name, arya::Duration cycle_time);

    /**
     * @brief CTOR
     *
     * @param[in] name Name of the job
     * @param[in] configuration Configuration of the job
     */
    inline JobInfo(const std::string& name, const arya::JobConfiguration& configuration);

    /**
     * @brief DTOR
     */
    ~JobInfo() = default;

    /**
     * @brief Get the Name of the Job
     *
     * @return The name of the Job
     */
    inline std::string getName() const;

    /**
     * @brief Get the JobConfiguration
     *
     * @return @ref fep3::arya::JobConfiguration
     */
    inline arya::JobConfiguration getConfig() const;

    /**
     * @brief Reconfigure JobInfo by replacing its JobConfiguration.
     * @param[in] configuration The new JobConfiguration to replace the existing one.
     *
     * @return fep3::Result
     */
    Result reconfigure(const arya::JobConfiguration& configuration)
    {
        _configuration = configuration;

        return {};
    }

private:
    std::string _name;
    arya::JobConfiguration _configuration;
};

JobInfo::JobInfo(const std::string& name, arya::Duration cycle_time)
    : _name(name), _configuration(cycle_time)
{
}

JobInfo::JobInfo(const std::string& name, const arya::JobConfiguration& configuration)
    : _name(name), _configuration(std::move(configuration))
{
}

std::string JobInfo::getName() const
{
    return _name;
}

arya::JobConfiguration JobInfo::getConfig() const
{
    return _configuration;
}

} // namespace arya

namespace catelyn {
/**
 * @brief JobInfo contains the name of a job and the corresponding job configuration.
 *
 * Usually JobInfos are used to retrieve information regarding jobs registered at the job registry.
 */
class JobInfo {
public:
    /**
     * @brief CTOR
     *
     * @param[in] name Name of the job
     * @param[in] configuration Configuration of the job
     */
    inline JobInfo(const std::string& name,
                   std::unique_ptr<catelyn::JobConfiguration> configuration)
        : _name(name), _configuration(std::move(configuration))
    {
    }

    /**
     * @brief CTOR
     *
     * @param[in] name Name of the job
     * @param[in] cycle_time Cycle time of the job
     */
    inline JobInfo(const std::string& name, arya::Duration cycle_time)
        : JobInfo(name, std::make_unique<catelyn::ClockTriggeredJobConfiguration>(cycle_time))
    {
    }

    /**
     * @brief CTOR
     *
     * @param[in] name Name of the job
     * @param[in] configuration Configuration of the job
     */
    inline JobInfo(const std::string& name, const arya::JobConfiguration& configuration)
        : JobInfo(name, std::make_unique<catelyn::ClockTriggeredJobConfiguration>(configuration))
    {
    }

    /**
     * @brief Copy CTOR
     *
     * @param[in] other The other value to take the value from
     */
    inline JobInfo(const JobInfo& other) : _name(other._name), _configuration(other.getConfigCopy())
    {
    }

    /**
     * @brief Copy Assignment Operator
     *
     * @param[in] other The other value to take the value from
     * @return JobInfo& A copy of the other JobInfo
     */
    inline JobInfo& operator=(const JobInfo& other)
    {
        _name = other._name;
        _configuration = other.getConfigCopy();
        return *this;
    }

    /**
     * @brief Move CTOR
     *
     * @param[in] other The other value to take the value from
     */
    inline JobInfo(JobInfo&& other)
    {
        _name = std::move(other._name);
        _configuration = std::move(other._configuration);
    }

    /**
     * @brief Move Assignment Operator
     *
     * @param[in] other The other value to take the value from
     * @return JobInfo& A copy of the other JobInfo
     */
    inline JobInfo& operator=(JobInfo&& other)
    {
        _name = std::move(other._name);
        _configuration = std::move(other._configuration);
        return *this;
    }

    /**
     * @brief DTOR
     */
    ~JobInfo() = default;

    /**
     * @brief Get the Name of the Job
     *
     * @return std::string The name of the Job
     */
    inline std::string getName() const
    {
        return _name;
    }

    /**
     * @brief Get the JobConfiguration
     *
     * @return @ref fep3::arya::JobConfiguration
     */
    inline std::unique_ptr<catelyn::JobConfiguration> getConfigCopy() const
    {
        return _configuration->clone();
    }

    /**
     * @brief Get the JobConfiguration
     *
     * @return fep3::arya::JobConfiguration
     */
    inline const catelyn::JobConfiguration& getConfig() const
    {
        return *_configuration;
    }

    /**
     * @brief Reconfigure JobInfo by replacing its JobConfiguration.
     * @param[in] configuration The new JobConfiguration to replace the existing one.
     *
     * @return fep3::Result
     */
    Result reconfigure(std::unique_ptr<catelyn::JobConfiguration> configuration)
    {
        _configuration = std::move(configuration);

        return {};
    }

private:
    std::string _name;
    std::unique_ptr<catelyn::JobConfiguration> _configuration;
};
} // namespace catelyn

using catelyn::JobInfo;
} // namespace fep3
