/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "mock_default_job.h"

#include <fep3/core/default_job.h>

#include <gmock/gmock.h>

#include <string>

namespace fep3::stub {

inline const std::string stub_default_job_name = "StubDefaultJob";

class DefaultJobModifiableTransitionResult : public fep3::core::DefaultJob {
public:
    DefaultJobModifiableTransitionResult()
        : fep3::core::DefaultJob(stub_default_job_name), _job_transition_res(nullptr)
    {
    }
    DefaultJobModifiableTransitionResult(const fep3::Result& job_transition_res)
        : fep3::core::DefaultJob(stub_default_job_name), _job_transition_res(&job_transition_res)
    {
    }
    void createDataIOs(const fep3::arya::IComponents&,
                       fep3::core::IDataIOContainer&,
                       const fep3::catelyn::JobConfiguration&) override
    {
    }

    fep3::Result execute(fep3::Timestamp)
    {
        return fep3::Result{};
    }

    fep3::Result initialize(const fep3::arya::IComponents&) override
    {
        if (_job_transition_res) {
            return *_job_transition_res;
        }
        else {
            return fep3::Result{};
        }
    };

    fep3::Result start() override
    {
        if (_job_transition_res) {
            return *_job_transition_res;
        }
        else {
            return fep3::Result{};
        }
    }

private:
    const fep3::Result* _job_transition_res;
};

class DefaultJob : public fep3::mock::DefaultJob {
public:
    DefaultJob() : fep3::mock::DefaultJob(fep3::stub::stub_default_job_name)
    {
        using ::testing::Return;
        ON_CALL(*this, createDataIOs).WillByDefault(Return());
        ON_CALL(*this, execute).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, initialize).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, start).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, stop).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, deinitialize).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, registerPropertyVariables).WillByDefault(Return(fep3::Result{}));
        ON_CALL(*this, unregisterPropertyVariables).WillByDefault(Return(fep3::Result{}));
    }
};

} // namespace fep3::stub
