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

#include <helper/copy_file.h>
#include <fep3/core.h>

const std::string faulty_create_method_components_file_path_source = std::string(TEST_BUILD_DIR) + "/files/faulty_create_method.fep_components";
const std::string empty_components_file_path_source = std::string(TEST_BUILD_DIR) + "/files/empty.fep_components";

class MyElement : public fep3::core::ElementBase
{
public:
    MyElement() : ElementBase("test", "test_version")
    {
    }
};

/**
 * @detail Test that error is reported on stderr when there is no Service Bus Component
 * @req_id TODO
 */
TEST(StartupErrorHandlingTest, testParticipantCreationWithoutServiceBus)
{
    using namespace fep3;

    // use empty components file to provoke error
    ASSERT_TRUE(test::helper::copyFile
        (empty_components_file_path_source
        , std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components")
        );

    std::stringstream stderr_reference;
    stderr_reference
        << getString(fep3::LoggerSeverity::error)
        << " No Service Bus Component available; Participant will not be visible on the Service Bus"
        << std::endl
        ;

    // startup logging goes to stdout/stderr
    ::testing::internal::CaptureStdout();
    ::testing::internal::CaptureStderr();

    Participant part = createParticipant<core::ElementFactory<MyElement>>("test", "testversion", "testsystem");

    std::string captured_stdout = testing::internal::GetCapturedStdout();
    std::string captured_stderr = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(captured_stdout.empty());
    EXPECT_EQ(stderr_reference.str(), captured_stderr);
}

/**
 * @detail Test that error is reported on stderr when Component creation fails
 * @req_id TODO
 */
TEST(StartupErrorHandlingTest, testComponentCreationFailure)
{
    using namespace fep3;

    // use components file that refers to a non-existing plugin to provoke error
    ASSERT_TRUE(test::helper::copyFile
        (faulty_create_method_components_file_path_source
        , std::string(TEST_BUILD_DIR) + "/fep3_participant.fep_components")
        );

    std::stringstream stderr_reference;
    stderr_reference
        << getString(fep3::LoggerSeverity::error)
        << " Creating the Component failed"
        << std::endl
        ;

    // startup logging goes to stdout/stderr
    ::testing::internal::CaptureStdout();
    ::testing::internal::CaptureStderr();

    Participant test_participant = createParticipant<core::ElementFactory<MyElement>>("test", "testversion", "testsystem");
    test_participant.exec(); // note: this must not block because of the error in the create method of the component

    std::string captured_stdout = testing::internal::GetCapturedStdout();
    std::string captured_stderr = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(captured_stdout.empty());
    EXPECT_NE(std::string::npos, captured_stderr.find(stderr_reference.str()));
}
