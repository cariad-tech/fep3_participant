/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/fep3_errors.h>

#include <gmock/gmock.h> // ASSERT_THAT
#include <gtest/gtest.h>

#define ASSERT_FEP3_RESULT(actual, expected)                                                       \
    {                                                                                              \
        fep3::Result actual_result = fep3::Result(actual);                                         \
        fep3::Result expected_result = fep3::Result(expected);                                     \
        ASSERT_EQ(actual_result.getErrorCode(), expected_result.getErrorCode())                    \
            << "actual was " << actual_result.getErrorLabel() << " expected was "                  \
            << expected_result.getErrorLabel() << std::endl                                        \
            << "Error Description: " << actual_result.getDescription();                            \
    }

#define EXPECT_FEP3_RESULT(actual, expected)                                                       \
    {                                                                                              \
        fep3::Result actual_result = fep3::Result(actual);                                         \
        fep3::Result expected_result = fep3::Result(expected);                                     \
        EXPECT_EQ(actual_result.getErrorCode(), expected_result.getErrorCode())                    \
            << "actual was " << actual_result.getErrorLabel() << " expected was "                  \
            << expected_result.getErrorLabel() << std::endl                                        \
            << "Error Description: " << actual_result.getDescription();                            \
    }

#define ASSERT_FEP3_NOERROR(actual) ASSERT_FEP3_RESULT(actual, fep3::ERR_NOERROR)

#define EXPECT_FEP3_NOERROR(actual) EXPECT_FEP3_RESULT(actual, fep3::ERR_NOERROR)

#define ASSERT_NO_CV_TIMEOUT(result) ASSERT_EQ(result, std::cv_status::no_timeout);

#define ASSERT_FEP3_RESULT_WITH_MESSAGE(actual, expected, description_regex)                       \
    {                                                                                              \
        fep3::Result actual_result = fep3::Result(actual);                                         \
        fep3::Result expected_result = fep3::Result(expected);                                     \
        ASSERT_EQ(actual_result.getErrorCode(), expected_result.getErrorCode())                    \
            << "actual was " << actual_result.getErrorLabel() << " expected was "                  \
            << expected_result.getErrorLabel();                                                    \
        ASSERT_THAT(std::string(actual_result.getDescription()),                                   \
                    ::testing::MatchesRegex(description_regex));                                   \
    }

#define ASSERT_FEP3_RESULT_TEST_ARRANGE(expression)                                                \
    ASSERT_EQ(expression, fep3::ERR_NOERROR) << "test failed due to error during the arrange "     \
                                                "part";
