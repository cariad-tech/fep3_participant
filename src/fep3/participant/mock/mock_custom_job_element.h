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

#include <fep3/core/custom_job_element.h>

#include <gmock/gmock.h>

namespace fep3 {
namespace mock {
class CustomJobElement : public fep3::core::CustomJobElement {
public:
    CustomJobElement(const std::string& name) : fep3::core::CustomJobElement(name)
    {
    }

    MOCK_METHOD(std::string, getTypename, (), (const));
    MOCK_METHOD(std::string, getVersion, (), (const));
    MOCK_METHOD((std::tuple<fep3::Result, JobPtr, JobConfigPtr>), createJob, (), ());
    MOCK_METHOD(fep3::Result, destroyJob, (), ());
    MOCK_METHOD(fep3::Result, load, (const fep3::arya::IComponents&), ());
    MOCK_METHOD(void, unload, (const fep3::arya::IComponents&), ());
    MOCK_METHOD(fep3::Result, initialize, (const fep3::arya::IComponents&), ());
    MOCK_METHOD(void, deinitialize, (const fep3::arya::IComponents&), ());
    MOCK_METHOD(fep3::Result, run, (), ());
    MOCK_METHOD(void, stop, (), ());
};

} // namespace mock
} // namespace fep3
