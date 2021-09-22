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

#include <fep3/fep3_errors.h>
#include <fep3/core/element_base.h>
#include <fep3/fep3_participant_version.h>
#include "datajob.h"

#include <string>
#include <memory>

namespace fep3
{
namespace cpp
{
namespace arya
{
/**
 * @brief This Simple Element Type will create and add one single DataJob implementation.
 *
 * @tparam data_job_type Type of the DataJob to add to the Element
 */
template<typename data_job_type>
class DataJobElement : public core::arya::ElementBase
{

public:
    /**
     * CTOR
     *
     */
    DataJobElement()
        : core::arya::ElementBase("fep3::cpp::DataJobElement",
                      FEP3_PARTICIPANT_LIBRARY_VERSION_STR),
          _job(std::make_shared<data_job_type>()),
          _need_reset(true)
    {
    }

    /**
     * @brief CTOR
     *
     * @param job the job which will be run cyclically
     */
    DataJobElement(std::shared_ptr<data_job_type> job)
        : ElementBase("fep3::cpp::DataJobElement",
                      FEP3_PARTICIPANT_LIBRARY_VERSION_STR),
          _job(std::move(job)),
          _need_reset(true)
    {
    }

    fep3::Result load() override
    {
        if (_job)
        {
            const auto components = getComponents();
            if (components)
            {
                auto config_service = components->getComponent<fep3::arya::IConfigurationService>();
                if (config_service)
                {
                    FEP3_RETURN_IF_FAILED(_job->initConfiguration(*config_service));
                }

                return arya::addToComponents({ _job }, *components);
            }
            else
            {
                RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "components reference invalid");
            }
        }
        else
        {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "job was not initialized in element");
        }
    }

    void unload() override
    {
        if (_job)
        {
            arya::removeFromComponents({ _job }, *getComponents());
            _job->deinitConfiguration();
        }
    }

    void stop() override
    {
        if (_job)
        {
            _need_reset = true;
        }
    }

    fep3::Result run() override
    {
        if (_job && _need_reset)
        {
            _need_reset = false;
            return _job->reset();
        }
        return {};
    }

private:
    /// data job
    std::shared_ptr<arya::DataJob> _job;
    bool                     _need_reset;
};

}

using arya::DataJobElement;

} // namespace cpp
} // namespace fep3
