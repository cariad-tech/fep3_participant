/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/cpp.h>

#include <gmock/gmock.h>

using namespace fep3::core;
using namespace fep3::base;

class MyElement : public fep3::core::ElementBase {
public:
    MyElement() : fep3::core::ElementBase("element_type", "element_version")
    {
    }
};

void rerouteStdout()
{
    ::testing::internal::CaptureStdout();

    std::atexit([]() {
        // We cannot use any assertions inside a death test because it would only create a
        // meaningless exit code
        std::cerr << ::testing::internal::GetCapturedStdout();
    });
}

template <typename T>
void deathTestFunction(int argc, char const* const* argv, T& command_line_parser_factory)
{
    rerouteStdout();
    createParticipant<ElementFactory<MyElement>>(
        argc, argv, "test_version", command_line_parser_factory.create());
}

template <typename T>
void deathTestFunctionNoDefaults(int argc, char const* const* argv, T& command_line_parser_factory)
{
    rerouteStdout();
    createParticipant<ElementFactory<MyElement>>(
        argc, argv, "test_version", command_line_parser_factory.createNoDefaults());
}

template <typename T>
struct HelperType {
    std::unique_ptr<CommandLineParser> create() const
    {
        return fep3::core::CommandLineParserFactory::create(
            T{}, {"test_participant", "test_system", ""});
    }

    std::unique_ptr<CommandLineParser> createNoDefaults() const
    {
        return fep3::core::CommandLineParserFactory::create(T{});
    }
};

template <typename T>
class CommandLineTest : public testing::Test {
public:
    HelperType<T> _helper_type;
};

template <typename T>
class CommandLineDeathTest : public CommandLineTest<T> {
};

using MyTypes = ::testing::Types<clipp::parameter, clipp::group>;
TYPED_TEST_SUITE(CommandLineTest, MyTypes);
TYPED_TEST_SUITE(CommandLineDeathTest, MyTypes);

/**
 * @detail Test createParticipant if no command line arguments are given
 * @req_id
 */
TYPED_TEST(CommandLineTest, NoArguments)
{
    const char* const argv[] = {"myExecutable", nullptr};

    Participant part = createParticipant<ElementFactory<MyElement>>(
        1, argv, "test_version", this->_helper_type.create());
    EXPECT_EQ(part.getName(), "test_participant");
    EXPECT_EQ(part.getSystemName(), "test_system");
}

/**
 * @detail Test createParticipant if no default values are defined
 * @req_id
 */
TYPED_TEST(CommandLineTest, MandatoryArguments)
{
    const char* const argv[] = {
        "myExecutable", "--system_name", "mySystem", "--element_name", "myParticipant", nullptr};

    Participant part = createParticipant<ElementFactory<MyElement>>(5, argv, "test_version");
    EXPECT_EQ(part.getName(), "myParticipant");
    EXPECT_EQ(part.getSystemName(), "mySystem");
}

/**
 * @detail Test createParticipant if no default values are defined and no commandline arguments get
 * passed
 */
TYPED_TEST(CommandLineDeathTest, MissingMandatoryArguments)
{
    const char* const argv[] = {"myExecutable", nullptr};
    ASSERT_EXIT(deathTestFunctionNoDefaults(1, argv, this->_helper_type),
                ::testing::ExitedWithCode(1),
                "Error in command line: Missing required argument");
}

/**
 * @detail Test createParticipant when the showVersion option gets passed
 * @req_id
 */
TYPED_TEST(CommandLineDeathTest, ShowVersion)
{
    const char* const argv_short[] = {"myExecutable", "-v", nullptr};
    const char* const argv_long[] = {"myExecutable", "--version", nullptr};

    ASSERT_EXIT(deathTestFunction(2, argv_short, this->_helper_type),
                ::testing::ExitedWithCode(0),
                FEP3_PARTICIPANT_LIBRARY_VERSION_STR);
    ASSERT_EXIT(deathTestFunction(2, argv_long, this->_helper_type),
                ::testing::ExitedWithCode(0),
                FEP3_PARTICIPANT_LIBRARY_VERSION_STR);
}

/**
 * @detail Test createParticipant when the showHelp option gets passed
 * @req_id
 */
TYPED_TEST(CommandLineDeathTest, ShowHelp)
{
    const char* const argv_short[] = {"myExecutable", "-h", nullptr};
    const char* const argv_long[] = {"myExecutable", "--help", nullptr};
    const char* const argv_alt[] = {"myExecutable", "-?", nullptr};

    ASSERT_EXIT(deathTestFunction(2, argv_short, this->_helper_type),
                ::testing::ExitedWithCode(0),
                ::testing::AllOf(::testing::HasSubstr("-?, -h, --help"),
                                 ::testing::HasSubstr("display usage information")));

    ASSERT_EXIT(deathTestFunction(2, argv_long, this->_helper_type),
                ::testing::ExitedWithCode(0),
                ::testing::AllOf(::testing::HasSubstr("-?, -h, --help"),
                                 ::testing::HasSubstr("display usage information")));

    ASSERT_EXIT(deathTestFunction(2, argv_alt, this->_helper_type),
                ::testing::ExitedWithCode(0),
                ::testing::AllOf(::testing::HasSubstr("-?, -h, --help"),
                                 ::testing::HasSubstr("display usage information")));
}

/**
 * @detail Test createParticipant when an invalid argument gets passed
 * @req_id
 */
TYPED_TEST(CommandLineDeathTest, InvalidArgument)
{
    const char* const argv[] = {"myExecutable", "-x", nullptr};

    ASSERT_EXIT(deathTestFunction(2, argv, this->_helper_type),
                ::testing::ExitedWithCode(1),
                ::testing::HasSubstr("#1 -x ->"));
}

/**
 * @detail Test createParticipant when the participant name is set with the command line
 * @req_id
 */
TYPED_TEST(CommandLineTest, SetParticipantName)
{
    const char* const argv[] = {"myExecutable", "-n", "myParticipant", nullptr};

    Participant part = createParticipant<ElementFactory<MyElement>>(
        3, argv, "test_version", this->_helper_type.create());
    EXPECT_EQ(part.getName(), "myParticipant");
    EXPECT_EQ(part.getSystemName(), "test_system");
}

/**
 * @detail Test createParticipant when the participant name is set with the command line (long
 * option)
 * @req_id
 */
TYPED_TEST(CommandLineTest, SetParticipantNameLong)
{
    const char* const argv[] = {"myExecutable", "--name", "myParticipant", nullptr};

    Participant part = createParticipant<ElementFactory<MyElement>>(
        3, argv, "test_version", this->_helper_type.create());
    EXPECT_EQ(part.getName(), "myParticipant");
    EXPECT_EQ(part.getSystemName(), "test_system");
}

/**
 * @detail Test createParticipant when the system name is set with the command line
 * @req_id
 */
TYPED_TEST(CommandLineTest, SetSystemName)
{
    const char* const argv[] = {"myExecutable", "-s", "mySystem", nullptr};

    Participant part = createParticipant<ElementFactory<MyElement>>(
        3, argv, "test_version", this->_helper_type.create());
    EXPECT_EQ(part.getName(), "test_participant");
    EXPECT_EQ(part.getSystemName(), "mySystem");
}

/**
 * @detail Test createParticipant when the system name is set with the command line (long option)
 * @req_id
 */
TYPED_TEST(CommandLineTest, SetSystemNameLong)
{
    const char* const argv[] = {"myExecutable", "--system", "mySystem", nullptr};

    Participant part = createParticipant<ElementFactory<MyElement>>(
        3, argv, "test_version", this->_helper_type.create());
    EXPECT_EQ(part.getName(), "test_participant");
    EXPECT_EQ(part.getSystemName(), "mySystem");
}

/**
 * @detail Test createParticipant when more than one option gets set at the same time
 * @req_id
 */
TYPED_TEST(CommandLineTest, SetParticipantAndSystemName)
{
    const char* const argv[] = {"myExecutable", "-s", "mySystem", "-n", "myParticipant", nullptr};

    Participant part = createParticipant<ElementFactory<MyElement>>(
        5, argv, "test_version", this->_helper_type.create());
    EXPECT_EQ(part.getName(), "myParticipant");
    EXPECT_EQ(part.getSystemName(), "mySystem");
}

/**
 * @detail Test createParticipant when an user defined argument gets passed
 * @req_id
 */
TYPED_TEST(CommandLineTest, SetUserDefinedArgument)
{
    const char* const argv[] = {
        "myExecutable", "-s", "mySystem", "-n", "myParticipant", "--user", "fep", nullptr};

    std::string user{};
    auto lambda1 = [&]() -> decltype(auto) {
        auto cli = (clipp::option("-u", "--user") &
                    clipp::value("user", user).doc("Set the user of the participant"));
        return cli;
    };

    auto part = createParticipant<ElementFactory<MyElement>>(
        7,
        argv,
        "test_version",
        fep3::core::CommandLineParserFactory::create(lambda1(),
                                                     {"test_participant", "test_system", ""}));
    EXPECT_EQ(user, "fep");
    EXPECT_EQ(part.getName(), "myParticipant");
    EXPECT_EQ(part.getSystemName(), "mySystem");
}

/**
 * @detail Test createParticipant when an user defined argument gets passed to a lambda function
 * @req_id
 */
TYPED_TEST(CommandLineTest, SetUserDefinedLambdaArgument)
{
    const char* const argv[] = {"myExecutable",
                                "-s",
                                "mySystem",
                                "-n",
                                "myParticipant",
                                "--identifier",
                                "42,cat,69,banana",
                                nullptr};

    std::vector<std::string> id_list{};
    auto lambda1 = [&]() -> decltype(auto) {
        const auto lambda = [&](const char* s) { id_list = a_util::strings::split(s, ","); };

        auto cli = (clipp::option("-i", "--identifier") &
                    clipp::value("identifier")
                        .call(lambda)
                        .doc("Set a list of comma separated identifiers"));
        return cli;
    };

    auto part = createParticipant<ElementFactory<MyElement>>(
        7,
        argv,
        "test_version",
        fep3::core::CommandLineParserFactory::create(lambda1(),
                                                     {"test_participant", "test_system", ""}));
    EXPECT_EQ(id_list[0], "42");
    EXPECT_EQ(id_list[1], "cat");
    EXPECT_EQ(id_list[2], "69");
    EXPECT_EQ(id_list[3], "banana");
    EXPECT_EQ(part.getName(), "myParticipant");
    EXPECT_EQ(part.getSystemName(), "mySystem");
}

/**
 * @detail Test createParticipant templates of the cpp library to make sure they compile correctly
 * @req_id
 */
TYPED_TEST(CommandLineTest, CppLibrary)
{
    const char* const argv[] = {"myExecutable", "-n", "myParticipant", nullptr};

    Participant part_1 =
        fep3::cpp::createParticipant<MyElement>(3, argv, "test_participant", "test_system");
    EXPECT_EQ(part_1.getName(), "myParticipant");

    clipp::group parser;
    Participant part_2 = fep3::cpp::createParticipant<MyElement>(
        3,
        argv,
        fep3::core::CommandLineParserFactory::create(parser,
                                                     {"test_participant", "test_system", ""}));
    EXPECT_EQ(part_2.getName(), "myParticipant");
}
