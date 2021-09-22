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

    ///@copydoc fep3::arya::addDataIn
    inline std::unique_ptr<fep3::arya::IDataRegistry::IDataReader> addDataIn(fep3::arya::IDataRegistry& data_registry,
        const std::string& name,
        const fep3::arya::IStreamType& stream_type,
        size_t queue_capacity = 1)
    {
        auto res = data_registry.registerDataIn(name, stream_type);
        if (isFailed(res))
        {
            return{};
        }
        else
        {
            return data_registry.getReader(name,
                                           queue_capacity);
        }
    }

    ///@copydoc fep3::arya::addDataOut
    inline std::unique_ptr<fep3::arya::IDataRegistry::IDataWriter> addDataOut(fep3::arya::IDataRegistry& data_registry,
        const std::string& name,
        const fep3::arya::IStreamType& stream_type,
        size_t queue_capacity = 0)
    {
        auto res = data_registry.registerDataOut(name, stream_type);
        if (isFailed(res))
        {
            return{};
        }
        else
        {
            return data_registry.getWriter(name, queue_capacity);
        }
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
