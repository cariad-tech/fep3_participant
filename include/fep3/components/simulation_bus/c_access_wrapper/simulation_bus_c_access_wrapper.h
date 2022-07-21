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

#include <cstring> // for strcmp
#include <functional>

#include <fep3/fep3_macros.h>
#include <fep3/components/simulation_bus/c_intf/simulation_bus_c_intf.h>
#include <fep3/base/stream_type/c_intf/stream_type_c_intf.h>
#include <fep3/base/stream_type/c_access_wrapper/stream_type_c_access_wrapper.h>
#include <fep3/components/simulation_bus/simulation_bus_intf.h>
#include <fep3/plugin/c/c_access/c_access_helper.h>
#include <fep3/plugin/c/c_wrapper/c_wrapper_helper.h>
#include <fep3/plugin/c/c_wrapper/destructor_c_wrapper.h>
#include <fep3/plugin/c/shared_binary_intf.h>
#include <fep3/components/base/component.h>
#include <fep3/components/base/c_access_wrapper/component_base_c_access.h>
#include <fep3/components/base/c_access_wrapper/component_c_wrapper.h>
#include <fep3/base/sample/c_access_wrapper/data_sample_c_access_wrapper.h>

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
 * @brief Access class for @ref fep3::arya::ISimulationBus.
 * Use this class to access a remote object of a type derived from @ref fep3::arya::ISimulationBus that resides in another binary (e. g. a shared library).
 */
class SimulationBus
    : public ::fep3::plugin::c::access::arya::ComponentBase<fep3::arya::ISimulationBus>
{
public:
    /// Symbol name of the create function that is capable to create a simulation bus
    static constexpr const char* const create_function_name = FEP3_EXPAND_TO_STRING(SYMBOL_fep3_plugin_c_arya_createSimulationBus);
    /// Gets the function to get an instance of a simulation bus that resides in a C plugin
    static decltype(&fep3_plugin_c_arya_getSimulationBus) getGetterFunction()
    {
         return fep3_plugin_c_arya_getSimulationBus;
    }
    /// Type of access object
    using Access = fep3_arya_SISimulationBus;

    /**
     * CTOR
     *
     * @param[in] access Access to the remote object
     * @param[in] shared_binary Shared pointer to the binary this resides in
     */
    inline SimulationBus
        (const Access& access
        , const std::shared_ptr<c::arya::ISharedBinary>& shared_binary
        );
    /**
     * DTOR destroying the corresponding remote object
     */
    inline ~SimulationBus() override = default;

    /**
     * Class wrapping access to the C interface for @ref fep3::arya::ISimulationBus::IDataReader.
     * Use this class to access a remote object of a type derived from @ref fep3::arya::ISimulationBus::IDataReader that resides in another binary (e. g. a shared library).
     */
    class DataReader
        : public fep3::arya::ISimulationBus::IDataReader
        , private c::arya::DestructionManager
    {
    public:
        /// Type of access structure
        using Access = fep3_arya_ISimulationBus_SIDataReader;

        /**
         * @brief CTOR
         * @param[in] access Access to the remote object
         * @param[in] destructors List of destructors to be called upon destruction of this
         */
        inline DataReader
            (const Access& access
            , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
            );
        inline ~DataReader() override = default;

        /// @cond no_documentation
        // methods implementing fep3::arya::IDataReader
        inline size_t size() const override;
        inline size_t capacity() const override;
        inline bool pop(IDataReceiver& receiver) override;
        inline void reset(const std::shared_ptr<IDataReceiver>& receiver) override;
        inline fep3::arya::Optional<fep3::arya::Timestamp> getFrontTime() const override;
        /// @endcond no_documentation

    private:
        Access _access;
    };

    /**
     * Class wrapping access to the C interface for @ref fep3::arya::ISimulationBus::IDataReceiver.
     * Use this class to access a remote object of a type derived from @ref fep3::arya::ISimulationBus::IDataReceiver that resides in another binary (e. g. a shared library).
     */
    class DataReceiver
        : public fep3::arya::ISimulationBus::IDataReceiver
        , private c::arya::DestructionManager
    {
    public:
        /// Type of access structure
        using Access = fep3_arya_ISimulationBus_SIDataReceiver;

        /**
         * @brief CTOR
         * @param[in] access Access to the remote object
         * @param[in] destructors List of destructors to be called upon destruction of this
         */
        inline DataReceiver
            (const Access& access
            , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
            );
        inline ~DataReceiver() override = default;

        /// @cond no_documentation
        // methods implementing fep3::arya::IDataReceiver
        inline void operator()(const data_read_ptr<const fep3::arya::IStreamType>& type) override;
        inline void operator()(const data_read_ptr<const fep3::arya::IDataSample>& sample) override;
        /// @endcond no_documentation

    private:
        Access _access;
    };

    /**
     * Class wrapping access to the C interface for @ref fep3::arya::ISimulationBus::IDataWriter.
     * Use this class to access a remote object of a type derived from @ref fep3::arya::ISimulationBus::IDataWriter
     * that resides in another binary (e. g. a shared library).
     */
    class DataWriter
        : public fep3::arya::ISimulationBus::IDataWriter
        , private c::arya::DestructionManager
    {
    public:
        /// Type of access structure
        using Access = fep3_arya_ISimulationBus_SIDataWriter;

        /**
         * @brief CTOR
         * @param[in] access Access to the remote object
         * @param[in] destructors List of destructors to be called upon destruction of this
         */
        inline DataWriter
            (const Access& access
            , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
            );
        inline ~DataWriter() override = default;

        /// @cond no_documentation
        // methods implementing fep3::arya::IDataWriter
        inline fep3::Result write(const fep3::arya::IDataSample& data_sample) override;
        inline fep3::Result write(const fep3::arya::IStreamType& stream_type) override;
        inline fep3::Result transmit() override;
        /// @endcond no_documentation

    private:
        Access _access;
    };

    /// @cond no_documentation
    // methods implementing fep3::arya::ISimulationBus
    inline bool isSupported(const fep3::arya::IStreamType& stream_type) const override;
    inline std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> getReader
        (const std::string& name
        , const fep3::arya::IStreamType& stream_type
        ) override;
    inline std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> getReader
        (const std::string& name
        , const fep3::arya::IStreamType& stream_type
        , size_t queue_capacity
        ) override;
    inline std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> getReader(const std::string& name) override;
    inline std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> getReader(const std::string& name, size_t queue_capacity) override;
    inline std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> getWriter
        (const std::string& name,
        const fep3::arya::IStreamType& stream_type
        ) override;
    inline std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> getWriter(const std::string& name, const fep3::arya::IStreamType& stream_type, size_t queue_capacity) override;
    inline std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> getWriter(const std::string& name) override;
    inline std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> getWriter(const std::string& name, size_t queue_capacity) override;
    inline void startBlockingReception(const std::function<void()>& reception_preparation_done_callback) override;
    inline void stopBlockingReception() override;
    /// @endcond no_documentation

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
 * Wrapper class for interface @ref fep3::arya::ISimulationBus
 */
class SimulationBus : private arya::Helper<fep3::arya::ISimulationBus>
{
public:
    /**
     * Wrapper class for interface @ref fep3::arya::ISimulationBus::IDataReader
     */
    class DataReader : private arya::Helper<fep3::arya::ISimulationBus::IDataReader>
    {
    public:
        /**
         * Functor creating an access structure for @ref ::fep3::arya::ISimulationBus::IDataReader
         */
        struct AccessCreator
        {
            /**
             * Creates an access structure to the data reader as pointed to by @p pointer_to_data_reader
             *
             * @param[in] pointer_to_data_reader Pointer to the data reader to create an access structure for
             * @return Access structure to the data reader
             */
            fep3_arya_ISimulationBus_SIDataReader operator()(fep3::arya::ISimulationBus::IDataReader* pointer_to_data_reader) const noexcept
            {
                return fep3_arya_ISimulationBus_SIDataReader
                    {reinterpret_cast<SimulationBus::DataReader::Handle>(pointer_to_data_reader)
                    , SimulationBus::DataReader::size
                    , SimulationBus::DataReader::capacity
                    , SimulationBus::DataReader::pop
                    , SimulationBus::DataReader::reset
                    , SimulationBus::DataReader::getFrontTime
                    };
            }
        };

        /// Alias for the helper
        using Helper = arya::Helper<fep3::arya::ISimulationBus::IDataReader>;
        /// Alias for the type of the handle to a wrapped object of type \ref fep3::arya::ISimulationBus::IDataReader
        using Handle = fep3_arya_ISimulationBus_HIDataReader;

        /// @cond no_documentation
        // static methods transferring calls from the C interface to an object of fep3::arya::ISimulationBus::IDataReader
        static inline fep3_plugin_c_InterfaceError size(Handle handle, size_t* result) noexcept
        {
            return Helper::callWithResultParameter
                (handle
                , &fep3::arya::ISimulationBus::IDataReader::size
                , [](const size_t& size)
                    {
                        return size;
                    }
                , result
                );
        }
        static inline fep3_plugin_c_InterfaceError capacity(Handle handle, size_t* result) noexcept
        {
            return Helper::callWithResultParameter
                (handle
                , &fep3::arya::ISimulationBus::IDataReader::capacity
                , [](const size_t& size)
                    {
                        return size;
                    }
                , result
                );
        }
        static inline fep3_plugin_c_InterfaceError pop
            (Handle handle
            , bool* result
            , fep3_arya_ISimulationBus_SIDataReceiver data_receiver_access
            ) noexcept
        {
            return passReferenceWithResultParameter<access::arya::SimulationBus::DataReceiver>
                (handle
                , [](auto&& data_reader, auto&& receiver)
                    {
                        return data_reader->pop(std::forward<decltype(receiver)>(receiver));
                    }
                , [](bool result)
                    {
                        return result;
                    }
                , result
                , data_receiver_access
                );
        }
        static inline fep3_plugin_c_InterfaceError reset
            (Handle handle
            , fep3_plugin_c_arya_SDestructionManager reference_manager_access
            , fep3_arya_ISimulationBus_SIDataReceiver data_receiver_access
            ) noexcept
        {
            return transferSharedPtr<access::arya::SimulationBus::DataReceiver>
                (handle
                , [](auto&& data_reader, auto&& receiver)
                    {
                        data_reader->reset(receiver);
                    }
                , reference_manager_access
                , data_receiver_access
                );
        }
        static inline fep3_plugin_c_InterfaceError getFrontTime
            (Handle handle
            , int64_t* result
            ) noexcept
        {
            return Helper::callWithResultParameter
                (handle
                , &fep3::arya::ISimulationBus::IDataReader::getFrontTime
                , [](const fep3::arya::Optional<fep3::arya::Timestamp>& timestamp)
                    {
                        // use min() as special value for invalid timestamp
                        return timestamp.value_or((fep3::arya::Timestamp::min)()).count();
                    }
                , result
                );
        }
        /// @endcond no_documentation
    };

    /**
     * Wrapper class for interface @ref fep3::arya::ISimulationBus::IDataReceiver
     */
    class DataReceiver : private arya::Helper<fep3::arya::ISimulationBus::IDataReceiver>
    {
    public:
        /**
         * Functor creating an access structure for @ref ::fep3::arya::ISimulationBus::IDataReceiver
         */
        struct AccessCreator
        {
            /**
             * Creates an access structure to the data receiver as pointed to by @p pointer_to_data_receiver
             *
             * @param[in] pointer_to_data_receiver Pointer to the data receiver to create an access structure for
             * @return Access structure to the data receiver
             */
            fep3_arya_ISimulationBus_SIDataReceiver operator()(fep3::arya::ISimulationBus::IDataReceiver* pointer_to_data_receiver) const noexcept
            {
                return fep3_arya_ISimulationBus_SIDataReceiver
                    {reinterpret_cast<wrapper::arya::SimulationBus::DataReceiver::Handle>(pointer_to_data_receiver)
                    , wrapper::arya::SimulationBus::DataReceiver::call
                    , wrapper::arya::SimulationBus::DataReceiver::call
                    };
            }
        };

        /// Alias for the helper
        using Helper = arya::Helper<fep3::arya::ISimulationBus::IDataReceiver>;
        /// Alias for the type of the handle to a wrapped object of type \ref fep3::arya::ISimulationBus::IDataReceiver
        using Handle = fep3_arya_ISimulationBus_HIDataReceiver;

        /// @cond no_documentation
        static inline fep3_plugin_c_InterfaceError call
            (Handle handle
            , fep3_plugin_c_arya_SDestructionManager reference_manager_access
            , fep3_arya_const_SIStreamType stream_type_access
            ) noexcept
        {
            return Helper::transferSharedPtr<access::arya::StreamType>
                (handle
                , [](auto&& data_receiver, auto&& stream_type)
                    {
                        return data_receiver->operator()(std::forward<decltype(stream_type)>(stream_type));
                    }
                , reference_manager_access
                , stream_type_access
                );
        }
        static inline fep3_plugin_c_InterfaceError call
            (Handle handle
            , fep3_plugin_c_arya_SDestructionManager reference_manager_access
            , fep3_arya_const_SIDataSample data_sample_access
            ) noexcept
        {
            return Helper::transferSharedPtr<access::arya::DataSample>
                (handle
                , [](auto&& data_receiver, auto&& data_sample)
                    {
                        return data_receiver->operator()(std::forward<decltype(data_sample)>(data_sample));
                    }
                , reference_manager_access
                , data_sample_access
                );
        }
        /// @endcond no_documentation
    };

    /**
     * Wrapper class for interface @ref fep3::arya::ISimulationBus::IDataWriter
     */
    class DataWriter : private arya::Helper<fep3::arya::ISimulationBus::IDataWriter>
    {
    public:
        /**
         * Functor creating an access structure for @ref ::fep3::arya::ISimulationBus::IDataWriter
         */
        struct AccessCreator
        {
            /**
             * Creates an access structure to the data writer as pointed to by @p pointer_to_data_writer
             *
             * @param[in] pointer_to_data_writer Pointer to the data writer to create an access structure for
             * @return Access structure to the data writer
             */
            fep3_arya_ISimulationBus_SIDataWriter operator()(fep3::arya::ISimulationBus::IDataWriter* pointer_to_data_writer) const noexcept
            {
                return fep3_arya_ISimulationBus_SIDataWriter
                    {reinterpret_cast<SimulationBus::DataWriter::Handle>(pointer_to_data_writer)
                    , DataWriter::writeDataSample
                    , DataWriter::writeStreamType
                    , DataWriter::transmit
                    };
            }
        };

        /// Alias for the helper
        using Helper = arya::Helper<fep3::arya::ISimulationBus::IDataWriter>;
        /// Alias for the type of the handle to a wrapped object of type \ref fep3::arya::ISimulationBus::IDataWriter
        using Handle = fep3_arya_ISimulationBus_HIDataWriter;

        /// @cond no_documentation
        static inline fep3_plugin_c_InterfaceError writeDataSample
            (Handle handle
            , fep3_result_callback_type result_callback
            , void* result_destination
            , fep3_arya_const_SIDataSample data_sample_access
            ) noexcept
        {
            return Helper::callWithResultCallback
                (handle
                // using static_cast to disambiguate the address of the appropriate overload
                , static_cast<fep3::Result(fep3::arya::ISimulationBus::IDataWriter::*)(const fep3::arya::IDataSample&)>
                    (&fep3::arya::ISimulationBus::IDataWriter::write)
                , result_callback
                , result_destination
                , getResult
                , access::arya::DataSample(data_sample_access, {})
                );
        }
        static inline fep3_plugin_c_InterfaceError writeStreamType
            (Handle handle
            , void(*result_callback)(void* destination, fep3_SResult result)
            , void* result_destination
            , fep3_arya_const_SIStreamType stream_type_access
            ) noexcept
        {
            return Helper::callWithResultCallback
                (handle
                // using static_cast to disambiguate the address of the appropriate overload
                , static_cast<fep3::Result(fep3::arya::ISimulationBus::IDataWriter::*)(const fep3::arya::IStreamType&)>
                    (&fep3::arya::ISimulationBus::IDataWriter::write)
                , result_callback
                , result_destination
                , getResult
                , access::arya::StreamType(stream_type_access, {})
                );
        }
        static inline fep3_plugin_c_InterfaceError transmit
            (Handle handle
            , fep3_result_callback_type result_callback
            , void* result_destination
            ) noexcept
        {
            return Helper::callWithResultCallback
                (handle
                , &fep3::arya::ISimulationBus::IDataWriter::transmit
                , result_callback
                , result_destination
                , getResult
                );
        }
        /// @endcond no_documentation
    };

private:
    using Helper = arya::Helper<fep3::arya::ISimulationBus>;
    using Handle = fep3_arya_HISimulationBus;

public:
    /// @cond no_documentation
    static inline fep3_plugin_c_InterfaceError isSupported
        (Handle handle
        , bool* result
        , fep3_arya_const_SIStreamType stream_type_access
        )
    {
        return Helper::callWithResultParameter
            (handle
            , &fep3::arya::ISimulationBus::isSupported
            , [](bool is_supported)
                {
                    return is_supported;
                }
            , result
            , access::arya::StreamType(stream_type_access, {})
            );
    }
    static inline fep3_plugin_c_InterfaceError getReaderByNameAndStreamType
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_ISimulationBus_SIDataReader* data_reader_access_result
        , const char* name
        , fep3_arya_const_SIStreamType stream_type_access
        )
    {
        return Helper::getUniquePtr
            (handle
            // using static_cast to disambiguate the address of the appropriate overload
            , static_cast<std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>(fep3::arya::ISimulationBus::*)(const std::string&, const fep3::arya::IStreamType&)>
                (&fep3::arya::ISimulationBus::getReader)
            , destruction_manager_access_result
            , data_reader_access_result
            , arya::SimulationBus::DataReader::AccessCreator()
            , name
            , access::arya::StreamType(stream_type_access, {})
            );
    }
    static inline fep3_plugin_c_InterfaceError getReaderByNameAndStreamTypeAndQueueCapacity
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_ISimulationBus_SIDataReader* data_reader_access_result
        , const char* name
        , fep3_arya_const_SIStreamType stream_type_access
        , size_t queue_capacity
        )
    {
        return Helper::getUniquePtr
            (handle
            // using static_cast to disambiguate the address of the appropriate overload
            , static_cast<std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>(fep3::arya::ISimulationBus::*)(const std::string&, const fep3::arya::IStreamType&, size_t)>
                (&fep3::arya::ISimulationBus::getReader)
            , destruction_manager_access_result
            , data_reader_access_result
            , arya::SimulationBus::DataReader::AccessCreator()
            , name
            , access::arya::StreamType(stream_type_access, {})
            , queue_capacity
            );
    }
    static inline fep3_plugin_c_InterfaceError getReaderByName
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_ISimulationBus_SIDataReader* data_reader_access_result
        , const char* name
        )
    {
        return Helper::getUniquePtr
            (handle
            // using static_cast to disambiguate the address of the appropriate overload
            , static_cast<std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>(fep3::arya::ISimulationBus::*)(const std::string&)>(&fep3::arya::ISimulationBus::getReader)
            , destruction_manager_access_result
            , data_reader_access_result
            , arya::SimulationBus::DataReader::AccessCreator()
            , name
            );
    }
    static inline fep3_plugin_c_InterfaceError getReaderByNameAndQueueCapacity
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_ISimulationBus_SIDataReader* data_reader_access_result
        , const char* name
        , size_t queue_capacity
        )
    {
        return Helper::getUniquePtr
            (handle
            // using static_cast to disambiguate the address of the appropriate overload
            , static_cast<std::unique_ptr<fep3::arya::ISimulationBus::IDataReader>(fep3::arya::ISimulationBus::*)(const std::string&, size_t)>(&fep3::arya::ISimulationBus::getReader)
            , destruction_manager_access_result
            , data_reader_access_result
            , arya::SimulationBus::DataReader::AccessCreator()
            , name
            , queue_capacity
            );
    }
    static inline fep3_plugin_c_InterfaceError getWriterByNameAndStreamType
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_ISimulationBus_SIDataWriter* data_writer_access_result
        , const char* name
        , fep3_arya_const_SIStreamType stream_type_access
        )
    {
        return Helper::getUniquePtr
            (handle
            // using static_cast to disambiguate the address of the appropriate overload
            , static_cast<std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>(fep3::arya::ISimulationBus::*)(const std::string&, const fep3::arya::IStreamType&)>
                (&fep3::arya::ISimulationBus::getWriter)
            , destruction_manager_access_result
            , data_writer_access_result
            , [](const auto& pointer_to_data_writer)
                {
                    return arya::SimulationBus::DataWriter::AccessCreator()(pointer_to_data_writer);
                }
            , name
            , access::arya::StreamType(stream_type_access, {})
            );
    }
    static inline fep3_plugin_c_InterfaceError getWriterByNameAndStreamTypeAndQueueCapacity
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_ISimulationBus_SIDataWriter* data_writer_access_result
        , const char* name
        , fep3_arya_const_SIStreamType stream_type_access
        , size_t queue_capacity
        )
    {
        return Helper::getUniquePtr
            (handle
            // using static_cast to disambiguate the address of the appropriate overload
            , static_cast<std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>(fep3::arya::ISimulationBus::*)(const std::string&, const IStreamType&, size_t)>
                (&fep3::arya::ISimulationBus::getWriter)
            , destruction_manager_access_result
            , data_writer_access_result
            , [](const auto& pointer_to_data_writer)
                {
                    return arya::SimulationBus::DataWriter::AccessCreator()(pointer_to_data_writer);
                }
            , name
            , access::arya::StreamType(stream_type_access, {})
            , queue_capacity
            );
    }
    static inline fep3_plugin_c_InterfaceError getWriterByName
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_ISimulationBus_SIDataWriter* data_writer_access_result
        , const char* name
        )
    {
        return Helper::getUniquePtr
            (handle
            // using static_cast to disambiguate the address of the appropriate overload
            , static_cast<std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>(fep3::arya::ISimulationBus::*)(const std::string&)>(&fep3::arya::ISimulationBus::getWriter)
            , destruction_manager_access_result
            , data_writer_access_result
            , [](const auto& pointer_to_data_writer)
                {
                    return arya::SimulationBus::DataWriter::AccessCreator()(pointer_to_data_writer);
                }
            , name
            );
    }
    static inline fep3_plugin_c_InterfaceError getWriterByNameAndQueueCapacity
        (Handle handle
        , fep3_plugin_c_arya_SDestructionManager* destruction_manager_access_result
        , fep3_arya_ISimulationBus_SIDataWriter* data_writer_access_result
        , const char* name
        , size_t queue_capacity
        )
    {
        return Helper::getUniquePtr
            (handle
            // using static_cast to disambiguate the address of the appropriate overload
            , static_cast<std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter>(fep3::arya::ISimulationBus::*)(const std::string&, size_t)>(&fep3::arya::ISimulationBus::getWriter)
            , destruction_manager_access_result
            , data_writer_access_result
            , [](const auto& pointer_to_data_writer)
                {
                    return arya::SimulationBus::DataWriter::AccessCreator()(pointer_to_data_writer);
                }
            , name
            , queue_capacity
            );
    }
    static inline fep3_plugin_c_InterfaceError startBlockingReception(Handle handle, void(*reception_preparation_done_callback)(const void* context), const void* context)
    {
        return Helper::call
            (handle
            , &fep3::arya::ISimulationBus::startBlockingReception
            , [reception_preparation_done_callback, context]()
                {
                    if(nullptr != reception_preparation_done_callback)
                    {
                        reception_preparation_done_callback(context);
                    }
                }
            );
    }
    static inline fep3_plugin_c_InterfaceError stopBlockingReception(Handle handle)
    {
        return Helper::call(handle, &fep3::arya::ISimulationBus::stopBlockingReception);
    }
    /// @endcond no_documentation

public:
    /// Type of access structure
    using Access = fep3_arya_SISimulationBus;
};

namespace detail
{

/// @cond no_documentation
inline fep3_plugin_c_InterfaceError getSimulationBus
    (fep3_arya_SISimulationBus* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    if(0 == strcmp(::fep3::arya::ISimulationBus::getComponentIID(), iid))
    {
        return ::fep3::plugin::c::wrapper::arya::get<::fep3::arya::IComponent, ::fep3::arya::ISimulationBus>
            (access_result
            , handle_to_component
            , [](::fep3::arya::ISimulationBus* pointer_to_object)
                {
                    return fep3_arya_SISimulationBus
                        {reinterpret_cast<fep3_arya_HISimulationBus>(pointer_to_object)
                        , {} // don't provide access to IComponent interface
                        , wrapper::arya::SimulationBus::isSupported
                        , wrapper::arya::SimulationBus::getReaderByNameAndStreamType
                        , wrapper::arya::SimulationBus::getReaderByNameAndStreamTypeAndQueueCapacity
                        , wrapper::arya::SimulationBus::getReaderByName
                        , wrapper::arya::SimulationBus::getReaderByNameAndQueueCapacity
                        , wrapper::arya::SimulationBus::getWriterByNameAndStreamType
                        , wrapper::arya::SimulationBus::getWriterByNameAndStreamTypeAndQueueCapacity
                        , wrapper::arya::SimulationBus::getWriterByName
                        , wrapper::arya::SimulationBus::getWriterByNameAndQueueCapacity
                        , wrapper::arya::SimulationBus::startBlockingReception
                        , wrapper::arya::SimulationBus::stopBlockingReception
                        };
                }
            );
    }
    else
    {
        // Note: not an error, this function is just not capable of getting the component for the passed IID
        return fep3_plugin_c_interface_error_none;
    }
}

template<typename factory_type>
inline fep3_plugin_c_InterfaceError createSimulationBus
    (factory_type&& factory
    , fep3_arya_SISimulationBus* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    using simulation_bus_type = typename std::remove_pointer<decltype(std::declval<factory_type>()())>::type;
    if(0 == strcmp(simulation_bus_type::getComponentIID(), iid))
    {
        return create
            (factory
            , result
            , shared_binary_access
            , [](simulation_bus_type* pointer_to_object)
                {
                    return fep3_arya_SISimulationBus
                        {reinterpret_cast<fep3_arya_HISimulationBus>(static_cast<fep3::arya::ISimulationBus*>(pointer_to_object))
                        , wrapper::arya::Component::Component::AccessCreator()(pointer_to_object)
                        , wrapper::arya::SimulationBus::isSupported
                        , wrapper::arya::SimulationBus::getReaderByNameAndStreamType
                        , wrapper::arya::SimulationBus::getReaderByNameAndStreamTypeAndQueueCapacity
                        , wrapper::arya::SimulationBus::getReaderByName
                        , wrapper::arya::SimulationBus::getReaderByNameAndQueueCapacity
                        , wrapper::arya::SimulationBus::getWriterByNameAndStreamType
                        , wrapper::arya::SimulationBus::getWriterByNameAndStreamTypeAndQueueCapacity
                        , wrapper::arya::SimulationBus::getWriterByName
                        , wrapper::arya::SimulationBus::getWriterByNameAndQueueCapacity
                        , wrapper::arya::SimulationBus::startBlockingReception
                        , wrapper::arya::SimulationBus::stopBlockingReception
                        };
                }
            );
    }
    else
    {
        // Note: not an error, this function is just not capable of creating the component for the passed IID
        return fep3_plugin_c_interface_error_none;
    }
}
/// @endcond no_documentation

} // namespace detail

/**
 * Creates a simulation bus object of type \p simulation_bus_type
 * @tparam simulation_bus_type The type of the simulation bus object to be created
 * @param[out] result Pointer to the access structure to the created simulation bus object
 * @param[in] shared_binary_access Access structure to the shared binary the simulation bus object resides in
 * @param[in] iid The interface ID of the simulation bus interface of the created object
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The @p result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while creating the component
 */
template<typename simulation_bus_type>
inline fep3_plugin_c_InterfaceError createSimulationBus
    (fep3_arya_SISimulationBus* result
    , const fep3_plugin_c_arya_SISharedBinary& shared_binary_access
    , const char* iid
    ) noexcept
{
    return detail::createSimulationBus
        ([]()
            {
                return new simulation_bus_type;
            }
        , result
        , shared_binary_access
        , iid
        );
}

} // namespace arya
} // namespace wrapper

namespace access
{
namespace arya
{

SimulationBus::DataReader::DataReader
    (const Access& access
    , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
    )
    : _access(std::move(access))
{
    addDestructors(std::move(destructors));
}

/// @cond no_documentation
size_t SimulationBus::DataReader::size() const
{
    return arya::Helper::callWithResultParameter(_access._handle, _access.size);
}

size_t SimulationBus::DataReader::capacity() const
{
    return arya::Helper::callWithResultParameter(_access._handle, _access.capacity);
}

bool SimulationBus::DataReader::pop(IDataReceiver& receiver)
{
    return arya::Helper::passReferenceWithResultParameter<bool>
        (receiver
        , _access._handle
        , _access.pop
        , [](const auto& pointer_to_receiver)
            {
                return ::fep3::plugin::c::wrapper::arya::SimulationBus::DataReceiver::AccessCreator()(pointer_to_receiver);
            }
        );
}

void SimulationBus::DataReader::reset(const std::shared_ptr<IDataReceiver>& receiver)
{
    arya::Helper::transferSharedPtr
        (receiver
        , _access._handle
        , _access.reset
        , [](const auto& pointer_to_receiver)
            {
                return ::fep3::plugin::c::wrapper::arya::SimulationBus::DataReceiver::AccessCreator()(pointer_to_receiver);
            }
        );
}

fep3::arya::Optional<fep3::arya::Timestamp> SimulationBus::DataReader::getFrontTime() const
{
    const auto next_time = arya::Helper::callWithResultParameter(_access._handle, _access.getFrontTime);
    if((fep3::arya::Timestamp::min)().count() == next_time)
    {
        return {};
    }
    else
    {
        return fep3::arya::Timestamp(next_time);
    }
}
/// @endcond no_documentation

SimulationBus::DataReceiver::DataReceiver
    (const Access& access
    , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
    )
    : _access(std::move(access))
{
    addDestructors(std::move(destructors));
}

/// @cond no_documentation
void SimulationBus::DataReceiver::operator()(const data_read_ptr<const fep3::arya::IStreamType>& type)
{
    arya::Helper::transferSharedPtr
        (type
        , _access._handle
        , _access.callByStreamType
        , [](const auto& pointer_to_type)
            {
                return ::fep3::plugin::c::wrapper::arya::StreamType::AccessCreator()(pointer_to_type);
            }
        );
}

void SimulationBus::DataReceiver::operator()(const data_read_ptr<const fep3::arya::IDataSample>& sample)
{
    arya::Helper::transferSharedPtr
        (sample
        , _access._handle
        , _access.callByDataSample
        , [](const auto& pointer_to_sample)
            {
                return ::fep3::plugin::c::wrapper::arya::DataSample::AccessCreator()(pointer_to_sample);
            }
        );
}
/// @endcond no_documentation

SimulationBus::DataWriter::DataWriter
    (const Access& access
    , std::deque<std::unique_ptr<c::arya::IDestructor>> destructors
    )
    : _access(std::move(access))
{
    addDestructors(std::move(destructors));
}

/// @cond no_documentation
fep3::Result SimulationBus::DataWriter::write(const fep3::arya::IDataSample& data_sample)
{
    return arya::Helper::callWithResultCallback<fep3::Result>
        (_access._handle
        , _access.writeDataSample
        , &getResult
        , wrapper::arya::DataSample::AccessCreator()(&data_sample)
        );
}

fep3::Result SimulationBus::DataWriter::write(const fep3::arya::IStreamType& stream_type)
{
    return arya::Helper::callWithResultCallback<fep3::Result>
        (_access._handle
        , _access.writeStreamType
        , &getResult
        , wrapper::arya::StreamType::AccessCreator()(&stream_type)
        );
}

fep3::Result SimulationBus::DataWriter::transmit()
{
    return arya::Helper::callWithResultCallback<fep3::Result>
        (_access._handle
        , _access.transmit
        , &getResult
        );
}
/// @endcond no_documentation

SimulationBus::SimulationBus
    (const Access& access
    , const std::shared_ptr<c::arya::ISharedBinary>& shared_binary
    )
    : arya::ComponentBase<fep3::arya::ISimulationBus>
        (access._component
        , shared_binary
        )
    , _access(std::move(access))
{}

/// @cond no_documentation
bool SimulationBus::isSupported(const fep3::arya::IStreamType& stream_type) const
{
    return arya::Helper::callWithResultParameter
        (_access._handle
        , _access.isSupported
        , ::fep3::plugin::c::wrapper::arya::StreamType::AccessCreator()(&stream_type)
        );
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader
    (const std::string& name
    , const fep3::arya::IStreamType& stream_type
    )
{
    return arya::Helper::getUniquePtr
        <DataReader
        , fep3_arya_ISimulationBus_SIDataReader
        >
        (_access._handle
        , _access.getReaderByNameAndStreamType
        , name.c_str()
        , ::fep3::plugin::c::wrapper::arya::StreamType::AccessCreator()(&stream_type)
        );
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader
    (const std::string& name
    , const fep3::arya::IStreamType& stream_type
    , size_t queue_capacity
    )
{
    return arya::Helper::getUniquePtr
        <DataReader
        , fep3_arya_ISimulationBus_SIDataReader
        >
        (_access._handle
        , _access.getReaderByNameAndStreamTypeAndQueueCapacity
        , name.c_str()
        , ::fep3::plugin::c::wrapper::arya::StreamType::AccessCreator()(&stream_type)
        , queue_capacity
        );
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader
    (const std::string& name)
{
    return arya::Helper::getUniquePtr
        <DataReader
        , fep3_arya_ISimulationBus_SIDataReader
        >
        (_access._handle, _access.getReaderByName, name.c_str());
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataReader> SimulationBus::getReader
    (const std::string& name
    , size_t queue_capacity
    )
{
    return arya::Helper::getUniquePtr
        <DataReader
        , fep3_arya_ISimulationBus_SIDataReader
        >
        (_access._handle, _access.getReaderByNameAndQueueCapacity, name.c_str(), queue_capacity);
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter(const std::string& name, const fep3::arya::IStreamType& stream_type)
{
    return arya::Helper::getUniquePtr
        <DataWriter
        , fep3_arya_ISimulationBus_SIDataWriter
        >
        (_access._handle
        , _access.getWriterByNameAndStreamType
        , name.c_str()
        , ::fep3::plugin::c::wrapper::arya::StreamType::AccessCreator()(&stream_type)
        );
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter
    (const std::string& name
    , const fep3::arya::IStreamType& stream_type
    , size_t queue_capacity
    )
{
    return arya::Helper::getUniquePtr
        <DataWriter
        , fep3_arya_ISimulationBus_SIDataWriter
        >
        (_access._handle
        , _access.getWriterByNameAndStreamTypeAndQueueCapacity
        , name.c_str()
        , ::fep3::plugin::c::wrapper::arya::StreamType::AccessCreator()(&stream_type)
        , queue_capacity
        );
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter(const std::string& name)
{
    return arya::Helper::getUniquePtr
        <DataWriter
        , fep3_arya_ISimulationBus_SIDataWriter
        >
        (_access._handle, _access.getWriterByName, name.c_str());
}

std::unique_ptr<fep3::arya::ISimulationBus::IDataWriter> SimulationBus::getWriter
    (const std::string& name
    , size_t queue_capacity
    )
{
    return arya::Helper::getUniquePtr
        <DataWriter
        , fep3_arya_ISimulationBus_SIDataWriter
        >
        (_access._handle, _access.getWriterByNameAndQueueCapacity, name.c_str(), queue_capacity);
}

inline void SimulationBus::startBlockingReception(const std::function<void()>& reception_preparation_done_callback)
{
    arya::Helper::call
        (_access._handle
        , _access.startBlockingReception
        , reception_preparation_done_callback ? static_cast<void(*)(const void*)>([](const void* context)
            {
                const auto& reception_preparation_done_callback = reinterpret_cast<const std::function<void()>*>(context);
                reception_preparation_done_callback->operator()();
            }) : nullptr
        , &reception_preparation_done_callback
        );
}

inline void SimulationBus::stopBlockingReception()
{
    arya::Helper::call(_access._handle, _access.stopBlockingReception);
}
/// @endcond no_documentation

} // namespace arya
} // namespace access
} // namespace c
} // namespace plugin
} // namespace fep3

/**
 * Gets access to a simulation bus object as identified by @p handle_to_component
 * @param[out] access_result Pointer to the access structure to the simulation bus object
 * @param[in] iid The interface ID of the simulation bus interface to get
 * @param[in] handle_to_component Handle to the interface of the object to get
 * @return Interface error code
 * @retval fep3_plugin_c_interface_error_none No error occurred
 * @retval fep3_plugin_c_interface_error_invalid_handle The \p handle_to_component is null
 * @retval fep3_plugin_c_interface_error_invalid_result_pointer The \p access_result is null
 * @retval fep3_plugin_c_interface_error_exception_caught An exception has been thrown while getting the component
 */
inline fep3_plugin_c_InterfaceError fep3_plugin_c_arya_getSimulationBus
    (fep3_arya_SISimulationBus* access_result
    , const char* iid
    , fep3_arya_HIComponent handle_to_component
    )
{
    return ::fep3::plugin::c::wrapper::arya::detail::getSimulationBus
        (access_result
        , iid
        , handle_to_component
        );
}
