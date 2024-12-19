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

#include <fep3/core/element_base.h>
#include <fep3/cpp/datajob.h>
#include <fep3/fep3_participant_version.h>

namespace fep3 {
namespace cpp {
/**
 * @brief This Simple Element Type will create and add one single DataJob implementation.
 *
 * @tparam data_job_type Type of the DataJob to add to the Element
 */
template <typename data_job_type>
class DataJobElement : public fep3::core::ElementBase {
protected:
    /**
     * @brief Default Copy CTOR
     */
    DataJobElement(const DataJobElement&) = default;

    /**
     * @brief Default Move CTOR
     */
    DataJobElement(DataJobElement&&) = default;

    /**
     * @brief Default Copy assignment operator
     *
     * @return DataJobElement&
     */
    DataJobElement& operator=(const DataJobElement&) = default;

    /**
     * @brief Default Move assignment operator
     *
     * @return DataJobElement&
     */
    DataJobElement& operator=(DataJobElement&&) = default;

public:
    /// Default DTOR
    ~DataJobElement() = default;

    /**
     * CTOR
     */
    DataJobElement()
        : fep3::core::ElementBase("fep3::cpp::DataJobElement",
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
        : ElementBase("fep3::cpp::DataJobElement", FEP3_PARTICIPANT_LIBRARY_VERSION_STR),
          _job(std::move(job)),
          _need_reset(true)
    {
    }

    fep3::Result load() override
    {
        if (_job) {
            const auto components = getComponents();
            if (components) {
                auto config_service = components->getComponent<fep3::IConfigurationService>();
                if (config_service) {
                    FEP3_RETURN_IF_FAILED(_job->initConfiguration(*config_service));
                }

                return addToComponents({_job}, *components);
            }
            else {
                RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "components reference invalid");
            }
        }
        else {
            RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "job was not initialized in element");
        }
    }

    void unload() override
    {
        if (_job) {
            removeFromComponents({_job}, *getComponents());
            _job->deinitConfiguration();
        }
    }

    void stop() override
    {
        if (_job) {
            _need_reset = true;
        }
    }

    fep3::Result run() override
    {
        if (_job && _need_reset) {
            _need_reset = false;
            return _job->reset();
        }
        return {};
    }

private:
    /// data job
    std::shared_ptr<DataJob> _job;
    bool _need_reset;
};

/// @cond nodoc
namespace arya {
template <typename data_job_type>
using DataJobElement [[deprecated("Since 3.1, fep3::arya::cpp::DataJobElement is deprecated. "
                                  "Please use fep3::cpp::DataJobElement")]] =
    fep3::cpp::DataJobElement<data_job_type>;
} // namespace arya
/// @endcond nodoc

} // namespace cpp
} // namespace fep3
