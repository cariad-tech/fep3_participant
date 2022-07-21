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

#include <memory>
#include <string>

#include "fep3/components/data_registry/data_registry_intf.h"
#include "fep3/fep3_errors.h"

namespace fep3
{
namespace base
{
namespace arya
{

    /**
     * @brief Helper function to register data to a given registry and create a reader immediately.
     *
     * @param[in,out] data_registry The data registry to register the incoming data to
     * @param[out] reader The returned reference is only valid when the function returns ERR_NOERROR
     * @param[in] name The name of the incoming data (must be unique)
     * @param[in] stream_type The stream type of this data (see @ref fep3::arya::IStreamType)
     * @param[in] queue_capacity The maximum number of items that the reader queue can hold at a time
     * @return fep3::Result ERR_NOERROR if succeeded, otherwise error code indicating occurred error
     *
     */
    inline fep3::Result addDataIn(fep3::arya::IDataRegistry& data_registry,
        std::unique_ptr<fep3::arya::IDataRegistry::IDataReader>& reader,
        const std::string& name,
        const fep3::arya::IStreamType& stream_type,
        size_t queue_capacity = 1)
    {
        fep3::Result res = data_registry.registerDataIn(name, stream_type);
        reader = isFailed(res) ? nullptr : data_registry.getReader(name, queue_capacity);
        return res;
    }

    /**
     * @brief Helper function to register data to a given registry and create a writer immediately.
     *
     * @param[in,out] data_registry The data registry to register the outgoing data to
     * @param[out] writer The returned reference is only valid when the function returns ERR_NOERROR
     * @param[in] name The name of the outgoing data (must be unique)
     * @param[in] stream_type The stream type of this data (see @ref fep3::arya::IStreamType)
     * @param[in] queue_capacity The maximum number of items that the transmit queue can hold at a time.
     * @return fep3::Result ERR_NOERROR if succeeded, otherwise error code indicating occurred error
     */
    inline fep3::Result addDataOut(fep3::arya::IDataRegistry& data_registry,
        std::unique_ptr<fep3::arya::IDataRegistry::IDataWriter>& writer,
        const std::string& name,
        const fep3::arya::IStreamType& stream_type,
        size_t queue_capacity = 0)
    {
        fep3::Result res = data_registry.registerDataOut(name, stream_type);
        writer = isFailed(res) ? nullptr : data_registry.getWriter(name, queue_capacity);
        return res;
    }

    /**
     * @brief Helper function to remove a data reader from a given registry.
     *
     * @param[in] data_registry The data registry to remove the data reader from
     * @param[in] name The name of the data reader
     * @return fep3::Result ERR_NOERROR if succeeded, error code otherwise:
     * @retval fep3::ERR_NOT_FOUND No data reader with this name found
     */
    inline fep3::Result removeDataIn(fep3::arya::IDataRegistry& data_registry,
        const std::string& name)
    {
        return data_registry.unregisterDataIn(name);
    }

    /**
     * @brief Helper function to remove a data writer from a given registry.
     *
     * @param[in] data_registry The data registry to remove the data writer from
     * @param[in] name The name of the data writer
     * @return fep3::Result ERR_NOERROR if succeeded, error code otherwise:
     * @retval fep3::ERR_NOT_FOUND No data writer with this name found
     */
    inline fep3::Result removeDataOut(fep3::arya::IDataRegistry& data_registry,
        const std::string& name)
    {
        return data_registry.unregisterDataOut(name);
    }
} // namespace arya
using arya::addDataIn;
using arya::addDataOut;
using arya::removeDataIn;
using arya::removeDataOut;
} // namespace base
} // namespace fep3
