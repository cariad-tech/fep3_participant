/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2021 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
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
} // namespace fep3

///@endcond nodoc