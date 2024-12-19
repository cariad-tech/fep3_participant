/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/core/step_unwinding.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <common/gtest_asserts.h>

using namespace fep3::core::detail;
using namespace testing;

namespace {

struct functor {
    void operator()()
    {
        _function_called = true;
    }

    bool _function_called = false;
};

struct functorWithRet {
    fep3::Result operator()()
    {
        _function_called = true;
        return _ret;
    }
    fep3::Result _ret{};
    bool _function_called = false;
};

struct functorMock {
    void operator()()
    {
        functionCall();
    }
    MOCK_METHOD(void, functionCall, (), (const));
};

struct functorWithRetMock {
    fep3::Result operator()()
    {
        functionCall();
        return _ret;
    }
    fep3::Result _ret{};
    MOCK_METHOD(void, functionCall, (), (const));
};

} // namespace

TEST(TestUnwinding, doTransition_oneAction_success)
{
    functorWithRet functor;
    Transition transition;
    transition.addAction(functor, []() {});

    transition.doTransition();

    ASSERT_TRUE(functor._function_called);
}

TEST(TestUnwinding, doTransition_oneActionFails_noUnwind)
{
    functorWithRet functor_action{fep3::Result{-1}};
    functor functor_undo;
    Transition transition;
    transition.addAction(functor_action, functor_undo);

    transition.doTransition();

    ASSERT_FALSE(functor_undo._function_called);
}

TEST(TestUnwinding, doTransition_fiveActions_success)
{
    std::array<functorWithRet, 5> functor_action;
    std::array<functor, 5> functor_undo;

    Transition transition;
    transition.addAction(functor_action[0], functor_undo[0]);
    transition.addAction(functor_action[1], functor_undo[1]);
    transition.addAction(functor_action[2], functor_undo[2]);
    transition.addAction(functor_action[3], functor_undo[3]);
    transition.addAction(functor_action[4], functor_undo[4]);

    transition.doTransition();

    ASSERT_THAT(functor_action, Each(Field(&functorWithRet::_function_called, true)));
    ASSERT_THAT(functor_undo, Each(Field(&functor::_function_called, false)));
}

TEST(TestUnwinding, doTransition_fiveActionsFirstFails_noUnwind)
{
    std::array<functorWithRet, 5> functor_action;
    std::array<functor, 5> functor_undo;

    functor_action[0]._ret = fep3::Result{-3};

    Transition transition;
    transition.addAction(functor_action[0], functor_undo[0]);
    transition.addAction(functor_action[1], functor_undo[1]);
    transition.addAction(functor_action[2], functor_undo[2]);
    transition.addAction(functor_action[3], functor_undo[3]);
    transition.addAction(functor_action[4], functor_undo[4]);
    transition.doTransition();

    ASSERT_THAT(functor_action,
                ElementsAre(Field(&functorWithRet::_function_called, true),
                            Field(&functorWithRet::_function_called, false),
                            Field(&functorWithRet::_function_called, false),
                            Field(&functorWithRet::_function_called, false),
                            Field(&functorWithRet::_function_called, false)));

    ASSERT_THAT(functor_undo, Each(Field(&functor::_function_called, false)));
}

TEST(TestUnwinding, doTransition_fiveActionsThirdFails_firstTwoUnwind)
{
    std::array<functorWithRet, 5> functor_action;
    std::array<functor, 5> functor_undo;

    functor_action[2]._ret = fep3::Result{-3};

    Transition transition;
    transition.addAction(functor_action[0], functor_undo[0]);
    transition.addAction(functor_action[1], functor_undo[1]);
    transition.addAction(functor_action[2], functor_undo[2]);
    transition.addAction(functor_action[3], functor_undo[3]);
    transition.addAction(functor_action[4], functor_undo[4]);
    transition.doTransition();

    ASSERT_THAT(functor_action,
                ElementsAre(Field(&functorWithRet::_function_called, true),
                            Field(&functorWithRet::_function_called, true),
                            Field(&functorWithRet::_function_called, true),
                            Field(&functorWithRet::_function_called, false),
                            Field(&functorWithRet::_function_called, false)));

    ASSERT_THAT(functor_undo,
                ElementsAre(Field(&functor::_function_called, true),
                            Field(&functor::_function_called, true),
                            Field(&functor::_function_called, false),
                            Field(&functor::_function_called, false),
                            Field(&functor::_function_called, false)));
}

TEST(TestUnwinding, doTransition_fiveActionsLastFails_allExceptLastUnwind)
{
    std::array<functorWithRet, 5> functor_action;
    std::array<functor, 5> functor_undo;

    functor_action[4]._ret = fep3::Result{-3};

    Transition transition;
    transition.addAction(functor_action[0], functor_undo[0]);
    transition.addAction(functor_action[1], functor_undo[1]);
    transition.addAction(functor_action[2], functor_undo[2]);
    transition.addAction(functor_action[3], functor_undo[3]);
    transition.addAction(functor_action[4], functor_undo[4]);
    transition.doTransition();

    ASSERT_THAT(functor_action,
                ElementsAre(Field(&functorWithRet::_function_called, true),
                            Field(&functorWithRet::_function_called, true),
                            Field(&functorWithRet::_function_called, true),
                            Field(&functorWithRet::_function_called, true),
                            Field(&functorWithRet::_function_called, true)));

    ASSERT_THAT(functor_undo,
                ElementsAre(Field(&functor::_function_called, true),
                            Field(&functor::_function_called, true),
                            Field(&functor::_function_called, true),
                            Field(&functor::_function_called, true),
                            Field(&functor::_function_called, false)));
}

TEST(TestUnwinding, doTransition_fiveActionsSuccess_returnNoError)
{
    std::array<functorWithRet, 5> functor_action;
    std::array<functor, 5> functor_undo;

    Transition transition;
    transition.addAction(functor_action[0], functor_undo[0]);
    transition.addAction(functor_action[1], functor_undo[1]);
    transition.addAction(functor_action[2], functor_undo[2]);
    transition.addAction(functor_action[3], functor_undo[3]);
    transition.addAction(functor_action[4], functor_undo[4]);

    ASSERT_FEP3_NOERROR(transition.doTransition());
}

TEST(TestUnwinding, doTransition_fiveActionsError_returnsErrorCode)
{
    std::array<functorWithRet, 5> functor_action;
    std::array<functor, 5> functor_undo;

    functor_action[4]._ret = fep3::Result{-3};

    Transition transition;
    transition.addAction(functor_action[0], functor_undo[0]);
    transition.addAction(functor_action[1], functor_undo[1]);
    transition.addAction(functor_action[2], functor_undo[2]);
    transition.addAction(functor_action[3], functor_undo[3]);
    transition.addAction(functor_action[4], functor_undo[4]);

    ASSERT_FEP3_RESULT(transition.doTransition(), fep3::Result{-3});
}

TEST(TestUnwinding, doTransition_ActionVoidReturnType_called)
{
    std::array<functorWithRet, 5> functor_action;
    std::array<functor, 5> functor_undo;

    functor action_without_ret;
    Transition transition;
    transition.addAction(functor_action[0], functor_undo[0]);
    transition.addAction(functor_action[1], functor_undo[1]);
    transition.addAction(functor_action[2], functor_undo[2]);
    transition.addAction(action_without_ret, functor_undo[2]);
    transition.addAction(functor_action[3], functor_undo[3]);
    transition.addAction(functor_action[4], functor_undo[4]);

    transition.doTransition();

    ASSERT_TRUE(action_without_ret._function_called);
}

TEST(TestUnwinding, doTransition_fiveActionsThirdFails_firstTwoUnwindReverseSequence)
{
    std::array<functorWithRetMock, 5> functor_action;
    std::array<functorMock, 5> functor_undo;

    functor_action[2]._ret = fep3::Result{-3};

    Transition transition;
    transition.addAction(functor_action[0], functor_undo[0]);
    transition.addAction(functor_action[1], functor_undo[1]);
    transition.addAction(functor_action[2], functor_undo[2]);
    transition.addAction(functor_action[3], functor_undo[3]);
    transition.addAction(functor_action[4], functor_undo[4]);

    {
        ::testing::InSequence s;
        EXPECT_CALL(functor_action[0], functionCall);
        EXPECT_CALL(functor_action[1], functionCall);
        EXPECT_CALL(functor_action[2], functionCall);
        EXPECT_CALL(functor_undo[1], functionCall);
        EXPECT_CALL(functor_undo[0], functionCall);

        transition.doTransition();
    }
}

TEST(TestUnwinding, doReverseTransition_oneAction_reverseActionCalled)
{
    functorWithRetMock functor_action;
    functorMock functor_undo;

    Transition transition;
    transition.addAction(functor_action, functor_undo);

    EXPECT_CALL(functor_action, functionCall).Times(0);
    EXPECT_CALL(functor_undo, functionCall);

    transition.doReverseTransition();
}

TEST(TestUnwinding, doReverseTransition_fiveActions_callsFunctionsInReverseOrder)
{
    std::array<functorWithRetMock, 5> functor_action;
    std::array<functorMock, 5> functor_undo;

    Transition transition;
    transition.addAction(functor_action[0], functor_undo[0]);
    transition.addAction(functor_action[1], functor_undo[1]);
    transition.addAction(functor_action[2], functor_undo[2]);
    transition.addAction(functor_action[3], functor_undo[3]);
    transition.addAction(functor_action[4], functor_undo[4]);

    {
        ::testing::InSequence s;
        EXPECT_CALL(functor_undo[4], functionCall);
        EXPECT_CALL(functor_undo[3], functionCall);
        EXPECT_CALL(functor_undo[2], functionCall);
        EXPECT_CALL(functor_undo[1], functionCall);
        EXPECT_CALL(functor_undo[0], functionCall);

        transition.doReverseTransition();
    }
}

TEST(TestUnwinding, doReverseTransition_noActionsAdded_noCrash)
{
    Transition transition;
    transition.doReverseTransition();
}

TEST(TestUnwinding, doTransition_noActionsAdded_noCrash)
{
    Transition transition;
    transition.doTransition();
}

TEST(TestUnwinding, doReverseTransition_fiveActionsTwoNoActions_threeActionsCalledInReverseSequence)
{
    std::array<functorWithRetMock, 5> functor_action;
    std::array<functorMock, 5> functor_undo;

    Transition transition;
    transition.addAction(functor_action[0], functor_undo[0]);
    transition.addAction(functor_action[1], NoAction{});
    transition.addAction(functor_action[2], NoAction{});
    transition.addAction(functor_action[3], functor_undo[3]);
    transition.addAction(functor_action[4], functor_undo[4]);

    {
        ::testing::InSequence s;
        EXPECT_CALL(functor_undo[4], functionCall);
        EXPECT_CALL(functor_undo[3], functionCall);
        EXPECT_CALL(functor_undo[0], functionCall);

        transition.doReverseTransition();
    }
}