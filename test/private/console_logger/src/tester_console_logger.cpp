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


#include <sstream>

#include <gtest/gtest.h>

#include <fep3/fep3_errors.h>
#include <fep3/participant/console_logger.h>

/**
 * Test all logging methods
 * @req_id TODO
 */
TEST(TestConsoleLogger, TestLogging)
{
    // enable all severites
    const auto& console_logger = fep3::ConsoleLogger(fep3::LoggerSeverity::debug);

    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();

    // order according to order of methods in ConsoleLogger class
    EXPECT_EQ(fep3::Result{}, console_logger.logInfo("Info"));
    EXPECT_EQ(fep3::Result{}, console_logger.logWarning("Warning"));
    EXPECT_EQ(fep3::Result{}, console_logger.logError("Error"));
    EXPECT_EQ(fep3::Result{}, console_logger.logFatal("Fatal"));
    EXPECT_EQ(fep3::Result{}, console_logger.logDebug("Debug"));

    std::string captured_stdout = testing::internal::GetCapturedStdout();
    std::string captured_stderr = testing::internal::GetCapturedStderr();

    // build the reference string for stdout
    std::stringstream stdout_reference;
    stdout_reference
        << getString(fep3::LoggerSeverity::info) << " Info" << std::endl
        << getString(fep3::LoggerSeverity::warning) << " Warning" << std::endl
        << getString(fep3::LoggerSeverity::debug) << " Debug" << std::endl
        ;
    // build the reference string for sterr
    std::stringstream stderr_reference;
    stderr_reference
        << getString(fep3::LoggerSeverity::error) << " Error" << std::endl
        << getString(fep3::LoggerSeverity::fatal) << " Fatal" << std::endl
        ;

    EXPECT_EQ(stdout_reference.str(), captured_stdout);
    EXPECT_EQ(stderr_reference.str(), captured_stderr);
}

/**
 * Test all severity methods
 * @req_id TODO
 */
TEST(TestConsoleLogger, TestSeverityMethods)
{
    {
        const auto& console_logger = fep3::ConsoleLogger(fep3::LoggerSeverity::off);

        EXPECT_FALSE(console_logger.isInfoEnabled());
        EXPECT_FALSE(console_logger.isWarningEnabled());
        EXPECT_FALSE(console_logger.isErrorEnabled());
        EXPECT_FALSE(console_logger.isFatalEnabled());
        EXPECT_FALSE(console_logger.isDebugEnabled());
    }

    {
        const auto& console_logger = fep3::ConsoleLogger(fep3::LoggerSeverity::fatal);

        EXPECT_FALSE(console_logger.isInfoEnabled());
        EXPECT_FALSE(console_logger.isWarningEnabled());
        EXPECT_FALSE(console_logger.isErrorEnabled());
        EXPECT_TRUE(console_logger.isFatalEnabled());
        EXPECT_FALSE(console_logger.isDebugEnabled());
    }

    {
        const auto& console_logger = fep3::ConsoleLogger(fep3::LoggerSeverity::error);

        EXPECT_FALSE(console_logger.isInfoEnabled());
        EXPECT_FALSE(console_logger.isWarningEnabled());
        EXPECT_TRUE(console_logger.isErrorEnabled());
        EXPECT_TRUE(console_logger.isFatalEnabled());
        EXPECT_FALSE(console_logger.isDebugEnabled());
    }

    {
        const auto& console_logger = fep3::ConsoleLogger(fep3::LoggerSeverity::warning);

        EXPECT_FALSE(console_logger.isInfoEnabled());
        EXPECT_TRUE(console_logger.isWarningEnabled());
        EXPECT_TRUE(console_logger.isErrorEnabled());
        EXPECT_TRUE(console_logger.isFatalEnabled());
        EXPECT_FALSE(console_logger.isDebugEnabled());
    }

    {
        const auto& console_logger = fep3::ConsoleLogger(fep3::LoggerSeverity::info);

        EXPECT_TRUE(console_logger.isInfoEnabled());
        EXPECT_TRUE(console_logger.isWarningEnabled());
        EXPECT_TRUE(console_logger.isErrorEnabled());
        EXPECT_TRUE(console_logger.isFatalEnabled());
        EXPECT_FALSE(console_logger.isDebugEnabled());
    }

    {
        const auto& console_logger = fep3::ConsoleLogger(fep3::LoggerSeverity::debug);

        EXPECT_TRUE(console_logger.isInfoEnabled());
        EXPECT_TRUE(console_logger.isWarningEnabled());
        EXPECT_TRUE(console_logger.isErrorEnabled());
        EXPECT_TRUE(console_logger.isFatalEnabled());
        EXPECT_TRUE(console_logger.isDebugEnabled());
    }
}
