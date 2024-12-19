/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/components/base/component.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>

#include <gmock/gmock.h>

///@cond nodoc

namespace fep3 {
namespace mock {
namespace arya {

namespace detail {

template <template <typename...> class component_base_type>
class SimulationBus : public component_base_type<fep3::arya::ISimulationBus> {
public:
    MOCK_METHOD(bool, isSupported, (const IStreamType&), (const, override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>,
                getReader,
                (const std::string&, const IStreamType&),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>,
                getReader,
                (const std::string&, const IStreamType&, size_t),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>,
                getReader,
                (const std::string&),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>,
                getReader,
                (const std::string&, size_t),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>,
                getWriter,
                (const std::string&, const IStreamType&),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>,
                getWriter,
                (const std::string&, const IStreamType&, size_t),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>,
                getWriter,
                (const std::string&),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>,
                getWriter,
                (const std::string&, size_t),
                (override));
    MOCK_METHOD(void, startBlockingReception, (const std::function<void()>&), (override));
    MOCK_METHOD(void, stopBlockingReception, (), (override));
};

} // namespace detail

class SimulationBus : public detail::SimulationBus<fep3::base::arya::Component> {
public:
    class DataReader : public fep3::arya::ISimulationBus::IDataReader {
    public:
        ~DataReader() override = default;
        MOCK_METHOD(size_t, size, (), (const, override));
        MOCK_METHOD(size_t, capacity, (), (const, override));
        MOCK_METHOD(bool, pop, (fep3::arya::ISimulationBus::IDataReceiver&), (override));
        // mocking a method with default parameter requires a helper
        void reset(const std::shared_ptr<fep3::arya::ISimulationBus::IDataReceiver>& receiver = {})
            override
        {
            reset_(receiver);
        }
        MOCK_METHOD(void,
                    reset_,
                    (const std::shared_ptr<fep3::arya::ISimulationBus::IDataReceiver>&));
        MOCK_METHOD(Optional<Timestamp>, getFrontTime, (), (const, override));
    };

    class DataReceiver : public fep3::arya::ISimulationBus::IDataReceiver {
    public:
        // mocking an operator requires a helper with regular name
        MOCK_METHOD(void, call, (const data_read_ptr<const IStreamType>&));
        virtual void operator()(const data_read_ptr<const IStreamType>& type) override
        {
            call(type);
        }
        // mocking an operator requires a helper with regular name
        MOCK_METHOD(void, call, (const data_read_ptr<const IDataSample>&));
        virtual void operator()(const data_read_ptr<const IDataSample>& sample) override
        {
            call(sample);
        }
    };

    class DataWriter : public fep3::arya::ISimulationBus::IDataWriter {
    public:
        MOCK_METHOD1(write, Result(const IDataSample&));
        MOCK_METHOD1(write, Result(const IStreamType&));
        MOCK_METHOD0(transmit, Result());
    };
};

} // namespace arya
using arya::SimulationBus;
} // namespace mock

namespace stub::arya {

struct SimulationBus : public fep3::base::arya::Component<fep3::arya::ISimulationBus> {
    struct DataReader : public fep3::arya::ISimulationBus::IDataReader {
        DataReader()
        {
            using namespace ::testing;
            ON_CALL(*this, size).WillByDefault(Return(1));
            ON_CALL(*this, capacity).WillByDefault(Return(1));
            ON_CALL(*this, pop).WillByDefault(Return(true));
            ON_CALL(*this, getFrontTime).WillByDefault(Return(fep3::arya::Timestamp{0}));
        }

        MOCK_METHOD(size_t, size, (), (const, override));
        MOCK_METHOD(size_t, capacity, (), (const, override));
        MOCK_METHOD(bool, pop, (fep3::arya::ISimulationBus::IDataReceiver&), (override));
        MOCK_METHOD(void,
                    reset,
                    (const std::shared_ptr<fep3::arya::ISimulationBus::IDataReceiver>&));
        MOCK_METHOD(Optional<Timestamp>, getFrontTime, (), (const, override));
    };

    struct DataReceiver : public fep3::arya::ISimulationBus::IDataReceiver {
    public:
        void operator()(const data_read_ptr<const IStreamType>&) override
        {
        }

        void operator()(const data_read_ptr<const IDataSample>&) override
        {
        }
    };

    struct DataWriter : public fep3::arya::ISimulationBus::IDataWriter {
    public:
        DataWriter()
        {
            using namespace ::testing;
            ON_CALL(*this, write(::testing::An<const fep3::arya::IStreamType&>()))
                .WillByDefault(Return(fep3::Result{}));
            ON_CALL(*this, write(::testing::An<const fep3::arya::IDataSample&>()))
                .WillByDefault(Return(fep3::Result{}));
            ON_CALL(*this, transmit).WillByDefault(Return(fep3::Result{}));
        }

        MOCK_METHOD(Result, write, (const IDataSample&), (override));
        MOCK_METHOD(Result, write, (const IStreamType&), (override));
        MOCK_METHOD(Result, transmit, (), (override));
    };

    SimulationBus()
    {
        using namespace testing;
        ON_CALL(*this, isSupported).WillByDefault(Return(true));

        ON_CALL(*this,
                getReader(::testing::An<const std::string&>(),
                          ::testing::An<const fep3::arya::IStreamType&>()))
            .WillByDefault(InvokeWithoutArgs([this]() { return getStubReader(); }));

        ON_CALL(*this, getReader(::testing::An<const std::string&>(), ::testing::An<size_t>()))
            .WillByDefault(InvokeWithoutArgs([this]() { return getStubReader(); }));

        ON_CALL(*this, getReader(_)).WillByDefault(InvokeWithoutArgs([this]() {
            return getStubReader();
        }));
        ON_CALL(*this, getReader(_, _, _)).WillByDefault(InvokeWithoutArgs([this]() {
            return getStubReader();
        }));

        ON_CALL(*this,
                getWriter(::testing::An<const std::string&>(),
                          ::testing::An<const fep3::arya::IStreamType&>()))
            .WillByDefault(InvokeWithoutArgs([this]() { return getStubWriter(); }));

        ON_CALL(*this, getWriter(::testing::An<const std::string&>(), ::testing::An<size_t>()))
            .WillByDefault(InvokeWithoutArgs([this]() { return getStubWriter(); }));

        ON_CALL(*this, getWriter(_)).WillByDefault(InvokeWithoutArgs([this]() {
            return getStubWriter();
        }));
        ON_CALL(*this, getWriter(_, _, _)).WillByDefault(InvokeWithoutArgs([this]() {
            return getStubWriter();
        }));
    }

    std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> getStubReader()
    {
        return std::make_unique<fep3::stub::arya::SimulationBus::DataReader>();
    }

    std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> getStubWriter()
    {
        return std::make_unique<fep3::stub::arya::SimulationBus::DataWriter>();
    }

    MOCK_METHOD(bool, isSupported, (const IStreamType&), (const, override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>,
                getReader,
                (const std::string&, const IStreamType&),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>,
                getReader,
                (const std::string&, const IStreamType&, size_t),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>,
                getReader,
                (const std::string&),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>,
                getReader,
                (const std::string&, size_t),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>,
                getWriter,
                (const std::string&, const IStreamType&),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>,
                getWriter,
                (const std::string&, const IStreamType&, size_t),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>,
                getWriter,
                (const std::string&),
                (override));
    MOCK_METHOD(std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>,
                getWriter,
                (const std::string&, size_t),
                (override));
    MOCK_METHOD(void, startBlockingReception, (const std::function<void()>&), (override));
    MOCK_METHOD(void, stopBlockingReception, (), (override));
};
} // namespace stub::arya

} // namespace fep3

///@endcond nodoc