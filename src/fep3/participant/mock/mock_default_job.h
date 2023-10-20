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

#pragma once

#include <fep3/core/default_job.h>

#include <gmock/gmock.h>

namespace fep3 {
namespace mock {

class DefaultJob : public fep3::core::DefaultJob {
public:
    DefaultJob(const std::string& name) : fep3::core::DefaultJob(name)
    {
    }

    MOCK_METHOD(void,
                createDataIOs,
                (const fep3::arya::IComponents&,
                 fep3::core::IDataIOContainer&,
                 const fep3::catelyn::JobConfiguration& config));
    MOCK_METHOD(fep3::Result, execute, (fep3::arya::Timestamp));
    MOCK_METHOD(fep3::Result, initialize, (const fep3::arya::IComponents&));
    MOCK_METHOD(fep3::Result, start, ());
    MOCK_METHOD(fep3::Result, stop, ());
    MOCK_METHOD(fep3::Result, registerPropertyVariables, ());
    MOCK_METHOD(fep3::Result, unregisterPropertyVariables, ());
};

} // namespace mock
} // namespace fep3
