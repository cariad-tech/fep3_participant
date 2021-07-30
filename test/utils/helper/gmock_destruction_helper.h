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


#pragma once

#include <gmock/gmock.h>

/**
 * Macro extending EXPECT_... macros of gmock by facilities
 * to check destruction of mock objects.
 *
 * Usage:
 * Derive your mock class from test::helper::Dieable:
 *
 * class MyMock : public test::helper::Dieable
 * {
 *     // ...
 * };
 *
 * Then in your test do:
 *
 * MyMock my_mock;
 * EXPECT_DESTRUCTION(my_mock);
 *
 * Of course the destructions of objects can be expected in a defined sequence:
 *
 * MyMock my_mock_1;
 * MyMock my_mock_2;
 * ::testing::InSequence sequence;
 * EXPECT_DESTRUCTION(my_mock_2);
 * EXPECT_DESTRUCTION(my_mock_1);
 *
 * To check destruction at specific check points, use:
 *
 * auto my_mock = std::make_unique<MyMock>;
 * auto destruction_checker = my_mock.getDestructionChecker();
 * EXPECT_DESTRUCTION(*my_mock.get());
 * // cause destruction
 * my_mock.reset();
 * // check point
 * ::testing::Mock::VerifyAndClearExpectations(destruction_checker.get())
 *
 * @note The underlying mock for destruction expectations is always a StrictMock.
 *
 * @details Background: Why do we need this helper? When using NiceMock/StrictMock
 *          the mocked class is the parent of NiceMock/StrictMock. Therefore, when the
 *          destructor of the mocked class is called, gmock is no longer aware of the
 *          NiceMock/StrictMock specification, but rather behaves like a NaggyMock
 *          (which reports uninteresting mock calls as warnings but does not cause the
 *          test to fail).
 *          With this helper, expectations for mock object destruction can be set,
 *          that lead to test failure if not satisfied.
 */
#define EXPECT_DESTRUCTION(mock_object_to_be_checked) \
EXPECT_CALL((mock_object_to_be_checked), die()) \
    .WillOnce(::testing::Invoke((mock_object_to_be_checked).getDestructionChecker().get(), &test::helper::detail::DestructionChecker::call)); \
EXPECT_CALL(*(mock_object_to_be_checked).getDestructionChecker().get(), call()).Times(1) \

namespace test
{
namespace helper
{
namespace detail
{

class DestructionChecker
{
public:
    DestructionChecker() = default;
    MOCK_CONST_METHOD0(call, void());
};

} // namespace detail

/**
 * @brief Class providing facilities to check destruction of a mock object
 * Derive your mock class publicly from this class and then define your expectations
 * for object destructions using @ref EXPECT_DESTRUCTION
 */
class Dieable
{
protected:
    virtual ~Dieable()
    {
        die();
    }
public:
    Dieable()
        : _mock_destruction_checker(std::make_shared<::testing::StrictMock<detail::DestructionChecker>>())
    {
        // note: if a mock class derives from Dieable, we don't want test failure if
        // no destruction expectations are set, so ignore any calls to die()
        EXPECT_CALL(*this, die()).Times(::testing::AnyNumber());
    }
    MOCK_METHOD0(die, void());
    std::shared_ptr<detail::DestructionChecker> getDestructionChecker()
    {
        return _mock_destruction_checker;
    }
private:
    std::shared_ptr<detail::DestructionChecker> _mock_destruction_checker;
};

} // namespace helper
} // namespace test