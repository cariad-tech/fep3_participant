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

#include <fep3/plugin/c/c_host_plugin.h>
#include <fep3/components/participant_info/mock/mock_participant_info.h>
#include <fep3/components/participant_info/c_access_wrapper/participant_info_c_access_wrapper.h>
#include <helper/component_c_plugin_helper.h>

const std::string test_plugin_1_path = PLUGIN;

using namespace fep3::plugin::c::arya;
using namespace fep3::plugin::c::access::arya;

struct Plugin1PathGetter
{
    std::string operator()() const
    {
        return test_plugin_1_path;
    }
};
struct SetMockComponentFunctionSymbolGetter
{
    std::string operator()() const
    {
        return "setMockParticipantInfo";
    }
};

/**
 * Test fixture loading a mocked participant info from within a C plugin
 */
using ParticipantInfoLoader = MockedComponentCPluginLoader
    <::fep3::IParticipantInfo
    , fep3::mock::ParticipantInfo<fep3::plugin::c::TransferableComponentBase>
    , ::fep3::plugin::c::access::arya::ParticipantInfo
    , Plugin1PathGetter
    , SetMockComponentFunctionSymbolGetter
    >;
using ParticipantInfoLoaderFixture = MockedComponentCPluginLoaderFixture<ParticipantInfoLoader>;

/**
 * Test method fep3::IParticipantInfo::getName of a participant info
 * that resides in a C plugin
 * @req_id FEPSDK-2570
 */
TEST_F(ParticipantInfoLoaderFixture, testMethod_getName)
{
    const auto& test_participant_name = std::string("test_participant");

    // setting of expectations
    {
        auto& mock_participant_info = getMockComponent();

        EXPECT_CALL(mock_participant_info, getName())
            .WillOnce(::testing::Return(test_participant_name));
    }
    ::fep3::arya::IParticipantInfo* participant_info = getComponent();
    ASSERT_NE(nullptr, participant_info);
    EXPECT_EQ(test_participant_name, participant_info->getName());
}

/**
 * Test method fep3::IParticipantInfo::getSystemName of a participant info
 * that resides in a C plugin
 * @req_id FEPSDK-2570
 */
TEST_F(ParticipantInfoLoaderFixture, testMethod_getSystemName)
{
    const auto& test_system_name = std::string("test_system");

    // setting of expectations
    {
        auto& mock_participant_info = getMockComponent();

        EXPECT_CALL(mock_participant_info, getSystemName())
            .WillOnce(::testing::Return(test_system_name));
    }
    ::fep3::arya::IParticipantInfo* participant_info = getComponent();
    ASSERT_NE(nullptr, participant_info);
    EXPECT_EQ(test_system_name, participant_info->getSystemName());
}