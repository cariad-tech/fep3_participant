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
// @note All methods are defined inline to provide the functionality as header only.

#pragma once

#include <fep3/base/sample/c_intf/data_sample_c_intf.h>
#include <fep3/base/sample/c_intf/raw_memory_c_intf.h>
#include <fep3/base/sample/data_sample_intf.h>
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
#include "raw_memory_c_access_wrapper.h"

namespace fep3
{
namespace plugin
{
namespace c
{
namespace access
{
namespace arya
{

/**
 * Class wrapping access to the C interface for @ref fep3::arya::IDataSample.
 * Use this class to access a remote object of a type derived from @ref fep3::arya::IDataSample that resides in another binary (e. g. a shared library).
 */
class DataSample
    : public fep3::arya::IDataSample
    , private c::arya::DestructionManager
    , private arya::Helper
{
public:
    /// Type of access structure
    using Access = fep3_arya_const_SIDataSample;

    /**
     * @brief CTOR
     * @param[in] access Access to the remote object
     * @param[in] destructors List of destructors to be called upon destruction of this
     */
    inline DataSample
        (const Access& access
        , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
        );
    inline ~DataSample() override = default;

    // methods implementing fep3::arya::IDataSample
    /**
     * Calls @ref fep3::arya::IDataSample::getTime(...) on the remote object
     * @return The timestamp as returned by call of @ref fep3::arya::IDataSample::getTime on the remote object
     */
    inline fep3::arya::Timestamp getTime() const override;
    /**
     * Calls @ref fep3::arya::IDataSample::getSize(...) on the remote object
     * @return The size as returned by call of @ref fep3::arya::IDataSample::getSize on the remote object
     */
    inline size_t getSize() const override;
    /**
     * Calls @ref fep3::arya::IDataSample::getCounter(...) on the remote object
     * @return The counter as returned by call of @ref fep3::arya::IDataSample::getCounter on the remote object
     */
    inline uint32_t getCounter() const override;
    /**
     * Calls @ref fep3::arya::IDataSample::read(...) on the remote object
     * @param[in,out] writeable_memory Reference to the writable memory to be passed
     * @return The size as returned by call of @ref fep3::arya::IDataSample::read on the remote object
     */
    inline size_t read(fep3::arya::IRawMemory& writeable_memory) const override;
private:
    // methods implementing non-const methods of fep3::arya::IDataSample
    /**
     * Calls @ref fep3::arya::IDataSample::setTime(...) on the remote object
     * @param[in] time The time to be set
     * @return The counter as returned by call of @ref fep3::arya::IDataSample::setTime on the remote object
     */
    inline void setTime(const fep3::arya::Timestamp& time) override;
    /**
     * Calls @ref fep3::arya::IDataSample::setCounter(...) on the remote object
     * @param[in] counter The counter to be set
     * @return The counter as returned by call of @ref fep3::arya::IDataSample::setCounter on the remote object
     */
    inline void setCounter(uint32_t counter) override;
    /**
     * Calls @ref fep3::arya::IDataSample::write(...) on the remote object
     * @param[in,out] readable_memory Reference to the readable memory to be passed
     * @return The counter as returned by call of @ref fep3::arya::IDataSample::write on the remote object
     */
    inline size_t write(const fep3::arya::IRawMemory& readable_memory) override;

private:
    Access _access;
};

} // namespace arya
} // namespace access

namespace wrapper
{
namespace arya
{

/**
 * Wrapper class for interface \ref fep3::arya::IDataSample
 */
class DataSample : private arya::Helper<const fep3::arya::IDataSample>
{
public:
    /**
     * Functor creating an access structure for @ref ::fep3::arya::IClock::IEventSink
     */
    struct AccessCreator
    {
        /**
         * Creates an access structure to the data sample as pointed to by @p pointer_to_data_sample
         *
         * @param[in] pointer_to_data_sample Pointer to the data sample to create an access structure for
         * @return Access structure to the data sample
         */
        fep3_arya_const_SIDataSample operator()(const fep3::arya::IDataSample* pointer_to_data_sample) const noexcept
        {
            return fep3_arya_const_SIDataSample
                {reinterpret_cast<fep3_arya_const_HIDataSample>(pointer_to_data_sample)
                , DataSample::getTime
                , DataSample::getSize
                , DataSample::getCounter
                , DataSample::read
                };
        }
    };

    /// Alias for the helper
    using Helper = arya::Helper<const fep3::arya::IDataSample>;
    /// Alias for the type of the handle to a wrapped object of type \ref fep3::arya::IDataSample
    using Handle = fep3_arya_const_HIDataSample;

    /**
     * Calls @ref fep3::arya::IDataSample::getTime(...) on the object identified by \p handle
     * @param[in] handle The handle to the object to call @ref fep3::arya::IDataSample::getTime on
     * @param[out] result Pointer to the result of the call of @ref fep3::arya::IDataSample::getTime
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IDataSample::getTime
     */
    static inline fep3_plugin_c_InterfaceError getTime(Handle handle, int64_t* result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IDataSample::getTime
            , [](const fep3::arya::Timestamp& timestamp)
                {
                    return timestamp.count();
                }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IDataSample::getSize(...) on the object identified by \p handle
     * @param[in] handle The handle to the object to call @ref fep3::arya::IDataSample::getSize on
     * @param[out] result Pointer to the result of the call of @ref fep3::arya::IDataSample::getSize
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IDataSample::getSize
     */
    static inline fep3_plugin_c_InterfaceError getSize(Handle handle, size_t* result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IDataSample::getSize
            , [](size_t size)
                {
                    return size;
                }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IDataSample::getCounter(...) on the object identified by \p handle
     * @param[in] handle The handle to the object to call @ref fep3::arya::IDataSample::getCounter on
     * @param[out] result Pointer to the result of the call of @ref fep3::arya::IDataSample::getCounter
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IDataSample::getCounter
     */
    static inline fep3_plugin_c_InterfaceError getCounter(Handle handle, uint32_t* result) noexcept
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IDataSample::getCounter
            , [](uint32_t counter)
                {
                    return counter;
                }
            , result
            );
    }
    /**
     * Calls @ref fep3::arya::IDataSample::read(...) on the object identified by \p handle
     * @param[in] handle The handle to the object to call @ref fep3::arya::IDataSample::read on
     * @param[in] result Pointer to the result of the call of @ref fep3::arya::IDataSample::read
     * @param[in] raw_memory_access The access structure to the raw memory to be passed
     * @return Interface error code
     * @retval fep3_plugin_c_interface_error_none No error occurred
     * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle is null
     * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p result is null
     * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown from within \ref fep3::arya::IDataSample::read
     */
    static inline fep3_plugin_c_InterfaceError read
        (Handle handle
        , size_t* result
        , fep3_arya_SIRawMemory raw_memory_access
        ) noexcept
    {
        access::arya::RawMemory raw_memory(raw_memory_access, {});
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::IDataSample::read
            , [](const size_t& size)
                {
                    return size;
                }
            , result
            , raw_memory
            );
    }
};

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

DataSample::DataSample
    (const Access& access
    , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
    )
    : _access(access)
{
    addDestructors(std::move(destructors));
}

fep3::arya::Timestamp DataSample::getTime() const
{
    return fep3::arya::Timestamp(callWithResultParameter(_access._handle, _access.getTime));
}

size_t DataSample::getSize() const
{
    return callWithResultParameter(_access._handle, _access.getSize);
}

uint32_t DataSample::getCounter() const
{
    return callWithResultParameter(_access._handle, _access.getCounter);
}

size_t DataSample::read(fep3::arya::IRawMemory& writeable_memory) const
{
    return Helper::callWithResultParameter
        (_access._handle
        , _access.read
        , fep3_arya_SIRawMemory
            {reinterpret_cast<fep3_arya_HIRawMemory>(&writeable_memory)
            , wrapper::arya::RawMemory::capacity
            , wrapper::arya::RawMemory::cdata
            , wrapper::arya::RawMemory::size
            , wrapper::arya::RawMemory::set
            , wrapper::arya::RawMemory::resize
            }
        );
}

void DataSample::setTime(const fep3::arya::Timestamp& /*time*/)
{
    throw Exception(fep3_plugin_c_interface_error_const_incorrectness);
}

void DataSample::setCounter(uint32_t /*counter*/)
{
    throw Exception(fep3_plugin_c_interface_error_const_incorrectness);
}

size_t DataSample::write(const fep3::arya::IRawMemory& /*readable_memory*/)
{
    throw Exception(fep3_plugin_c_interface_error_const_incorrectness);
}

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3
