/**
 * @file
 * @copyright
 * @verbatim
Copyright 2023 CARIAD SE.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#pragma once

#include <fep3/core/data_io_container_intf.h>

#include <list>

namespace fep3::core {

/// @cond nodoc
class DataIOContainer : public IDataIOContainer {
public:
    fep3::core::DataReader* addDataIn(
        const std::string& name,
        const fep3::arya::IStreamType& type,
        size_t queue_size,
        const fep3::catelyn::JobConfiguration& job_configuration) override;
    fep3::core::DataWriter* addDataOut(const std::string& name,
                                       const fep3::arya::IStreamType& type,
                                       size_t queue_size) override;
    fep3::Result executeDataIn(fep3::arya::Timestamp time_of_execution) override;
    fep3::Result executeDataOut(fep3::arya::Timestamp time_of_execution) override;
    fep3::Result addToDataRegistry(fep3::arya::IDataRegistry& data_registry,
                                   fep3::arya::IClockService& clock_service) override;
    void removeFromDataRegistry(fep3::arya::IDataRegistry& data_registry) override;
    void setConfiguration(const DataIOContainerConfiguration& configuration) override;

    void logIOInfo(const fep3::arya::ILogger* logger) override;

private:
    /// list of readers
    std::list<DataReader> _readers;
    /// list of writers
    std::list<DataWriter> _writers;
    /// capacity of storage containing information regarding purged samples
    /// configuration structure
    DataIOContainerConfiguration _configuration;
};
/// @endcond

} // namespace fep3::core
