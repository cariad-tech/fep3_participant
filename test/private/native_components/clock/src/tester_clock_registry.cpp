/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/base/mock_components.h>
#include <fep3/components/clock/clock_intf.h>
#include <fep3/components/clock/mock_clock.h>
#include <fep3/components/clock/mock_clock_service.h>
#include <fep3/components/logging/mock/mock_logger_addons.h>
#include <fep3/components/logging/mock_logging_service.h>
#include <fep3/native_components/clock/clock_registry.h>

#include <common/gtest_asserts.h>
#include <type_traits>

using namespace ::testing;
using namespace fep3::experimental;
using namespace fep3;

using ExperimentalClock = NiceMock<fep3::mock::experimental::Clock>;
using AryaClock = NiceMock<fep3::mock::arya::Clock>;
using Logger = NiceMock<fep3::mock::LoggerWithDefaultBehavior>;
using LoggingService = NiceMock<fep3::mock::LoggingService>;
using ComponentsMock = NiceMock<fep3::mock::Components>;

template <typename T>
struct ClockRegistryTest : public ::testing::Test {
    using MockClockType = T;
    using ClockInterfaceType = typename T::ClockInterfaceType;
    using ClockEventSinkInterfaceType = typename T::EventSink::ClockEventSinkInterfaceType;
    void SetUp() override
    {
        ON_CALL(*_clock1_mock.get(), getName()).WillByDefault(Return("my_clock1"));
        ON_CALL(*_clock2_mock.get(), getName()).WillByDefault(Return("my_clock2"));
        ON_CALL(*_native_clock1_mock.get(), getName()).WillByDefault(Return("native_1"));
        ON_CALL(*_native_clock2_mock.get(), getName()).WillByDefault(Return("my_native_2"));
        ON_CALL(*_logging_service_mock, createLogger(_)).WillByDefault(Return(_logger_mock));
        ON_CALL(*_components_mock.get(), findComponent(StrEq(LoggingService::getComponentIID())))
            .WillByDefault(Return(_logging_service_mock.get()));
        ;
        _clock_registry = std::make_unique<fep3::native::ClockRegistry>();
        _clock_registry->initLogger(*_components_mock, "clock_registry.component");
    }
    void TearDown() override
    {
        // nothing
    }

    template <typename S, typename U>
    void verifyClock(S findClockResult, U& clock_mock)
    {
        using ClockType = typename S::element_type;
        EXPECT_CALL(*clock_mock, start(_));
        EXPECT_CALL(*clock_mock, stop());
        EXPECT_CALL(*clock_mock, reset(fep3::Timestamp{69}));
        EXPECT_CALL(*clock_mock, getType())
            .WillOnce(Return(fep3::arya::IClock::ClockType::continuous));
        EXPECT_CALL(*clock_mock, getTime()).WillOnce(Return(fep3::Timestamp{25}));

        ASSERT_EQ(findClockResult->getName(), clock_mock->getName());

        if constexpr (std::is_same_v<ClockType, fep3::arya::IClock>) {
            findClockResult->start(std::shared_ptr<fep3::arya::IClock::IEventSink>(nullptr));
        }
        else {
            findClockResult->start(
                std::shared_ptr<fep3::experimental::IClock::IEventSink>(nullptr));
        }

        findClockResult->stop();
        findClockResult->reset(fep3::Timestamp{69});
        ASSERT_EQ(findClockResult->getType(), fep3::arya::IClock::ClockType::continuous);
        ASSERT_EQ(findClockResult->getTime(), fep3::Timestamp{25});
    }

    std::unique_ptr<fep3::native::ClockRegistry> _clock_registry{};
    std::shared_ptr<MockClockType> _clock1_mock = std::make_shared<MockClockType>();
    std::shared_ptr<MockClockType> _clock2_mock = std::make_shared<MockClockType>();
    std::shared_ptr<MockClockType> _native_clock1_mock = std::make_shared<MockClockType>();
    std::shared_ptr<MockClockType> _native_clock2_mock = std::make_shared<MockClockType>();
    std::shared_ptr<ILogger> _logger_mock = std::make_shared<Logger>();
    std::unique_ptr<LoggingService> _logging_service_mock = std::make_unique<LoggingService>();
    std::shared_ptr<ComponentsMock> _components_mock = std::make_shared<ComponentsMock>();
};

template <typename ClockType>
struct ClockRegistryWithTwoClocksTest : public ClockRegistryTest<ClockType> {
    void SetUp() override
    {
        ClockRegistryTest<ClockType>::SetUp();
        this->_clock_registry->registerClock(this->_clock1_mock);
        this->_clock_registry->registerClock(this->_clock2_mock);
    }
};

using ClockTypes = ::testing::Types<ExperimentalClock, AryaClock>;
TYPED_TEST_SUITE(ClockRegistryTest, ClockTypes);
TYPED_TEST_SUITE(ClockRegistryWithTwoClocksTest, ClockTypes);

struct ClockRegistryTestNativeClocks : ClockRegistryTest<ExperimentalClock> {
};

TEST_F(ClockRegistryTestNativeClocks, registerNativeClocks__successful)
{
    ASSERT_FEP3_NOERROR(
        _clock_registry->registerNativeClocks({_native_clock1_mock, _native_clock2_mock}));
}

TEST_F(ClockRegistryTestNativeClocks, registerNativeClocks__failOnFurtherCalls)
{
    _clock_registry->registerNativeClocks({_native_clock1_mock, _native_clock2_mock});
    ASSERT_FEP3_RESULT(_clock_registry->registerNativeClocks({_native_clock1_mock}),
                       fep3::ERR_INVALID_ARG);
    ASSERT_FEP3_RESULT(_clock_registry->registerNativeClocks({_clock2_mock}),
                       fep3::ERR_INVALID_ARG);
}

TEST_F(ClockRegistryTestNativeClocks, unregisterClock__failOnNativeClocks)
{
    _clock_registry->registerNativeClocks({_native_clock1_mock, _native_clock2_mock});
    ASSERT_FEP3_RESULT(_clock_registry->unregisterClock(_native_clock1_mock->getName()),
                       fep3::ERR_INVALID_ARG);
    ASSERT_FEP3_RESULT(_clock_registry->unregisterClock(_native_clock2_mock->getName()),
                       fep3::ERR_INVALID_ARG);
}

TYPED_TEST(ClockRegistryTest, registerClock__successful)
{
    ASSERT_FEP3_NOERROR(this->_clock_registry->registerClock(this->_clock1_mock));
}

TYPED_TEST(ClockRegistryTest, registerClock__failOnInvalidPointer)
{
    ASSERT_FEP3_RESULT(this->_clock_registry->template registerClock<fep3::arya::IClock>(nullptr),
                       fep3::ERR_POINTER);
    ASSERT_FEP3_RESULT(
        this->_clock_registry->template registerClock<fep3::experimental::IClock>(nullptr),
        fep3::ERR_POINTER);
}

TYPED_TEST(ClockRegistryTest, registerClock__failOnNameAlreadyExists)
{
    this->_clock_registry->registerClock(this->_clock1_mock);
    ASSERT_FEP3_RESULT(this->_clock_registry->registerClock(this->_clock1_mock),
                       fep3::ERR_INVALID_ARG);
}

TYPED_TEST(ClockRegistryTest, unregisterClock__successful)
{
    this->_clock_registry->registerClock(this->_clock1_mock);
    ASSERT_FEP3_NOERROR(this->_clock_registry->unregisterClock(this->_clock1_mock->getName()));
}

TYPED_TEST(ClockRegistryTest, unregisterClock__failOnNonExistingClock)
{
    this->_clock_registry->registerClock(this->_clock1_mock);
    ASSERT_FEP3_RESULT(this->_clock_registry->unregisterClock("non-existent-clock"),
                       fep3::ERR_INVALID_ARG);
}

TYPED_TEST(ClockRegistryWithTwoClocksTest, getClockNames__successful)
{
    const std::list<std::string> clock_list = {this->_clock1_mock->getName(),
                                               this->_clock2_mock->getName()};
    ASSERT_EQ(this->_clock_registry->getClockNames().size(), 2);
    ASSERT_EQ(this->_clock_registry->getClockNames(), clock_list);
}

TYPED_TEST(ClockRegistryWithTwoClocksTest, findClock__successful)
{
    using ThisType = std::remove_pointer_t<std::remove_cv_t<decltype(this)>>;
    using ClockInterfaceType = typename ThisType::ClockInterfaceType;
    ASSERT_EQ(this->_clock_registry->template findClock<ClockInterfaceType>(
                  this->_clock1_mock->getName()),
              this->_clock1_mock);
    ASSERT_EQ(this->_clock_registry->template findClock<ClockInterfaceType>(
                  this->_clock2_mock->getName()),
              this->_clock2_mock);
}

TYPED_TEST(ClockRegistryWithTwoClocksTest, findClock__failOnNonExistingClock)
{
    ASSERT_EQ(
        this->_clock_registry->template findClock<fep3::experimental::IClock>("non_existing_clock"),
        nullptr);
}

TYPED_TEST(ClockRegistryWithTwoClocksTest, findClockAsArya__successful)
{
    ASSERT_NO_FATAL_FAILURE(
        this->verifyClock(this->_clock_registry->template findClock<fep3::arya::IClock>(
                              this->_clock1_mock->getName()),
                          this->_clock1_mock));
    ASSERT_NO_FATAL_FAILURE(
        this->verifyClock(this->_clock_registry->template findClock<fep3::arya::IClock>(
                              this->_clock2_mock->getName()),
                          this->_clock2_mock));
}

TYPED_TEST(ClockRegistryWithTwoClocksTest, findClockAsExperimental__successful)
{
    ASSERT_NO_FATAL_FAILURE(
        this->verifyClock(this->_clock_registry->template findClock<fep3::experimental::IClock>(
                              this->_clock1_mock->getName()),
                          this->_clock1_mock));
    ASSERT_NO_FATAL_FAILURE(
        this->verifyClock(this->_clock_registry->template findClock<fep3::experimental::IClock>(
                              this->_clock2_mock->getName()),
                          this->_clock2_mock));
}

TYPED_TEST(ClockRegistryWithTwoClocksTest, getClockAdapter__failOnNonExistingClock)
{
    ASSERT_EQ(this->_clock_registry->getClockAdapter("non_existing_clock"), std::nullopt);
}

TYPED_TEST(ClockRegistryWithTwoClocksTest, getClockAdapter__successful)
{
    ASSERT_NE(this->_clock_registry->getClockAdapter(this->_clock1_mock->getName()), std::nullopt);
}
