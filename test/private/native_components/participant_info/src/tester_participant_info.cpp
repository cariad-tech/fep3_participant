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


#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fep3/participant/participant.h>
#include <fep3/cpp/element_base.h>
#include <fep3/components/participant_info/participant_info_intf.h>
#include <fep3/core/participant_executor.hpp>

using namespace ::testing;
using namespace fep3::arya;
using namespace fep3::core::arya;

#define TEST_PARTICIPANT_NAME "test_participant"
#define TEST_SYSTEM_NAME "test_system_name"

class TestElement : public fep3::core::ElementBase
{
public:
    TestElement() : fep3::core::ElementBase("", "")
    {

    }
};

class TestElementFactory : public ::fep3::IElementFactory
{
public:
    TestElementFactory()
    {
    }

    std::unique_ptr<::fep3::IElement> createElement(const ::fep3::IComponents& components) const override
    {
        auto participant_info = components.getComponent<IParticipantInfo>();

        EXPECT_EQ(participant_info->getName(), TEST_PARTICIPANT_NAME);
        EXPECT_EQ(participant_info->getSystemName(), TEST_SYSTEM_NAME);

        return std::make_unique<TestElement>();
    }
};

/**
* @brief Create participant with participant and system name.
* During IElementFactory::createElement we check the value are correctly provided by the IParticipantInfo.
* @req_id FEPSDK-2570
*/
TEST(ParticipantInfoTester, CheckParticipantNameAndSystemName)
{
    auto participant = fep3::createParticipant<TestElementFactory>(TEST_PARTICIPANT_NAME, "3.0.1", TEST_SYSTEM_NAME);
    fep3::core::ParticipantExecutor executor_sender(participant);
    executor_sender.exec();
    executor_sender.load();
    executor_sender.initialize();
}
