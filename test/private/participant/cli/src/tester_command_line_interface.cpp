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

#include <fep3/core.h>
#include <fep3/cpp.h>

using namespace fep3::core;

class MyElement : public fep3::core::ElementBase
{
public:
    MyElement() : ElementBase("element_type", "element_version")
    {
    }
};

void rerouteStdout()
{
    ::testing::internal::CaptureStdout();

    std::atexit([]()
    {
        // We cannot use any assertions inside a death test because it would only create a meaningless exit code
        std::cerr << ::testing::internal::GetCapturedStdout();
    });
}

void deathTestFunction(int argc, char const *const *argv)
{
    rerouteStdout();
    createParticipant<ElementFactory<MyElement>>(argc, argv, "test_version", { "test_participant", "test_system", "" });
}

void deathTestFunctionNoDefaults(int argc, char const *const *argv)
{
    rerouteStdout();
    createParticipant<ElementFactory<MyElement>>(argc, argv, "test_version");
}

/**
* @detail Test createParticipant if no command line arguments are given
* @req_id
*/
TEST(CommandLineTest, NoArguments)
{
    const char *const argv[] = { "myExecutable", nullptr };

    Participant part = createParticipant<ElementFactory<MyElement>>(1, argv, "test_version", { "test_participant", "test_system", "" });
    EXPECT_EQ(part.getName(), "test_participant");
    EXPECT_EQ(part.getSystemName(), "test_system");
}

/**
* @detail Test createParticipant if no default values are defined
* @req_id
*/
TEST(CommandLineTest, MandatoryArguments)
{
    const char *const argv[] = { "myExecutable", "myParticipant", "mySystem", nullptr };

    Participant part = createParticipant<ElementFactory<MyElement>>(3, argv, "test_version");
    EXPECT_EQ(part.getName(), "myParticipant");
    EXPECT_EQ(part.getSystemName(), "mySystem");
}

/**
* @detail Test createParticipant if no default values are defined and no commandline arguments get passed
*/
TEST(CommandLineDeathTest, MissingMandatoryArguments)
{
    const char *const argv[] = { "myExecutable", nullptr };

    ASSERT_EXIT(deathTestFunctionNoDefaults(1, argv), ::testing::ExitedWithCode(1), "Error in command line: Missing required argument");
}

/**
* @detail Test createParticipant when the showVersion option gets passed
* @req_id
*/
TEST(CommandLineDeathTest, ShowVersion)
{
    const char *const argv_short[] = { "myExecutable", "-v", nullptr };
    const char *const argv_long[] = { "myExecutable", "--version", nullptr };

    ASSERT_EXIT(deathTestFunction(2, argv_short), ::testing::ExitedWithCode(0), FEP3_PARTICIPANT_LIBRARY_VERSION_STR);
    ASSERT_EXIT(deathTestFunction(2, argv_long), ::testing::ExitedWithCode(0), FEP3_PARTICIPANT_LIBRARY_VERSION_STR);
}

/**
* @detail Test createParticipant when the showHelp option gets passed
* @req_id
*/
TEST(CommandLineDeathTest, ShowHelp)
{
    const char *const argv_short[] = { "myExecutable", "-h", nullptr };
    const char *const argv_long[] = { "myExecutable", "--help", nullptr };
    const char *const argv_alt[] = { "myExecutable", "-?", nullptr };

    const std::string help_text{ "usage:\\s+.+options\\s+where options are:\\s+-\\?, -h, --help\\s+display usage information" };

    ASSERT_EXIT(deathTestFunction(2, argv_short), ::testing::ExitedWithCode(0), help_text);
    ASSERT_EXIT(deathTestFunction(2, argv_long), ::testing::ExitedWithCode(0), help_text);
    ASSERT_EXIT(deathTestFunction(2, argv_alt), ::testing::ExitedWithCode(0), help_text);
}

/**
* @detail Test createParticipant when an invalid argument gets passed
* @req_id
*/
TEST(CommandLineDeathTest, InvalidArgument)
{
    const char *const argv[] = { "myExecutable", "-x", nullptr };

    ASSERT_EXIT(deathTestFunction(2, argv), ::testing::ExitedWithCode(1), "Error in command line: Unrecognised token: -x");
}

/**
* @detail Test createParticipant when the participant name is set with the command line
* @req_id
*/
TEST(CommandLineTest, SetParticipantName)
{
    const char *const argv[] = { "myExecutable", "-n", "myParticipant", nullptr };

    Participant part = createParticipant<ElementFactory<MyElement>>(3, argv, "test_version", { "test_participant", "test_system", "" });
    EXPECT_EQ(part.getName(), "myParticipant");
    EXPECT_EQ(part.getSystemName(), "test_system");
}

/**
* @detail Test createParticipant when the participant name is set with the command line (long option)
* @req_id
*/
TEST(CommandLineTest, SetParticipantNameLong)
{
    const char *const argv[] = { "myExecutable", "--name", "myParticipant", nullptr };

    Participant part = createParticipant<ElementFactory<MyElement>>(3, argv, "test_version", { "test_participant", "test_system", "" });
    EXPECT_EQ(part.getName(), "myParticipant");
    EXPECT_EQ(part.getSystemName(), "test_system");
}

/**
* @detail Test createParticipant when the system name is set with the command line
* @req_id
*/
TEST(CommandLineTest, SetSystemName)
{
    const char *const argv[] = { "myExecutable", "-s", "mySystem", nullptr };

    Participant part = createParticipant<ElementFactory<MyElement>>(3, argv, "test_version", { "test_participant", "test_system", "" });
    EXPECT_EQ(part.getName(), "test_participant");
    EXPECT_EQ(part.getSystemName(), "mySystem");
}

/**
* @detail Test createParticipant when the system name is set with the command line (long option)
* @req_id
*/
TEST(CommandLineTest, SetSystemNameLong)
{
    const char *const argv[] = { "myExecutable", "--system", "mySystem", nullptr };

    Participant part = createParticipant<ElementFactory<MyElement>>(3, argv, "test_version", { "test_participant", "test_system", "" });
    EXPECT_EQ(part.getName(), "test_participant");
    EXPECT_EQ(part.getSystemName(), "mySystem");
}

/**
* @detail Test createParticipant when more than one option gets set at the same time
* @req_id
*/
TEST(CommandLineTest, SetParticipantAndSystemName)
{
    const char *const argv[] = { "myExecutable", "-s", "mySystem", "-n", "myParticipant", nullptr };

    Participant part = createParticipant<ElementFactory<MyElement>>(5, argv, "test_version", { "test_participant", "test_system", "" });
    EXPECT_EQ(part.getName(), "myParticipant");
    EXPECT_EQ(part.getSystemName(), "mySystem");
}

/**
* @detail Test createParticipant when an user defined argument gets passed
* @req_id
*/
TEST(CommandLineTest, SetUserDefinedArgument)
{
    const char *const argv[] = { "myExecutable", "--user", "fep", nullptr };

    std::string user{};
    clara::Parser parser;
    parser |= clara::Opt(user, "string")
        ["-u"]["--user"]
        ("Set the user of the participant");

    createParticipant<ElementFactory<MyElement>>(3, argv, "test_version", fep3::CommandLineParserFactory::create(parser, { "test_participant", "test_system", "" }));
    EXPECT_EQ(user, "fep");
}

/**
* @detail Test createParticipant when an user defined argument gets passed to a lambda function
* @req_id
*/
TEST(CommandLineTest, SetUserDefinedLambdaArgument)
{
    const char *const argv[] = { "myExecutable", "--identifier", "42,cat,69,banana", nullptr};

    std::vector<std::string> id_list{};
    const auto lambda = [&](std::string s)
    {
        id_list = a_util::strings::split(s, ",");
    };

    clara::Parser parser;
    parser |= clara::Opt(lambda, "list")
        ["-i"]["--identifier"]
        ("Set a list of comma separated identifiers");

    createParticipant<ElementFactory<MyElement>>(3, argv, "test_version", fep3::CommandLineParserFactory::create(parser, { "test_participant", "test_system", "" }));
    EXPECT_EQ(id_list[0], "42");
    EXPECT_EQ(id_list[1], "cat");
    EXPECT_EQ(id_list[2], "69");
    EXPECT_EQ(id_list[3], "banana");
}

/**
* @detail Test createParticipant templates of the cpp library to make sure they compile correctly
* @req_id
*/
TEST(CommandLineTest, CppLibrary)
{
    const char *const argv[] = { "myExecutable", "-n", "myParticipant", nullptr };

    Participant part_1 = fep3::cpp::createParticipant<MyElement>(3, argv, "test_participant", "test_system");
    EXPECT_EQ(part_1.getName(), "myParticipant");

    clara::Parser parser;
    Participant part_2 = fep3::cpp::createParticipant<MyElement>(3, argv, fep3::CommandLineParserFactory::create(parser, { "test_participant", "test_system", "" }));
    EXPECT_EQ(part_2.getName(), "myParticipant");
}