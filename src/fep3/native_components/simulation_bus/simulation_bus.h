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

#include "fep3/components/simulation_bus/simulation_bus_intf.h"
#include "fep3/components/base/component.h"

#include "fep3/components/logging/logging_service_intf.h"

namespace fep3
{
namespace native
{

//simulation bus implementation supporting the arya service bus
class SimulationBus: public fep3::base::Component<fep3::arya::ISimulationBus>
{
public:
    SimulationBus();
    virtual ~SimulationBus();
    SimulationBus(const SimulationBus&) = delete;
    SimulationBus(SimulationBus&&) = delete;
    SimulationBus& operator=(const SimulationBus&) = delete;
    SimulationBus& operator=(SimulationBus&&) = delete;

public: // inherited via base::Component
    fep3::Result initialize() override;
    fep3::Result deinitialize() override;

public:
    bool isSupported(const IStreamType& stream_type) const override;

    std::unique_ptr<IDataReader> getReader(const std::string& name,
            const IStreamType& stream_type) override;

    std::unique_ptr<IDataReader> getReader(
            const std::string& name
            , const IStreamType& stream_type
            , size_t queue_capacity
            ) override;

    std::unique_ptr<IDataReader> getReader(const std::string& name) override;

    std::unique_ptr<IDataReader> getReader(const std::string& name, size_t queue_capacity) override;

    std::unique_ptr<IDataWriter> getWriter
       (const std::string& name
       , const IStreamType& stream_type
       ) override;

    std::unique_ptr<IDataWriter> getWriter
       (const std::string& name
       , const IStreamType& stream_type
       , size_t queue_capacity
       ) override;

    std::unique_ptr<IDataWriter> getWriter(const std::string& name) override;

    std::unique_ptr<IDataWriter> getWriter(const std::string& name, size_t queue_capacity) override;

    void startBlockingReception(const std::function<void()>& reception_preparation_done_callback) override;
    void stopBlockingReception() override;

    class Transmitter;
    class DataReader;
    class DataWriter;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace native
} // namespace fep3
