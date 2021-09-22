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

#include "fep3/base/sample/data_sample_intf.h"
#include "fep3/base/sample/raw_memory_intf.h"
#include "fep3/base/stream_type/stream_type_intf.h"
#include "fep3/components/base/component_iid.h"
#include "fep3/components/simulation_bus/simulation_bus_intf.h"
#include "fep3/fep3_errors.h"
#include "fep3/fep3_participant_types.h"

/**
* @brief The data registry main property tree entry node
*/
#define FEP3_DATA_REGISTRY_CONFIG "data_registry"

/**
* @brief The mapping configuration property name
* Use this to set the mapping configuration with a single string from inside the data registry configuration node.
*/
#define FEP3_MAPPING_CONFIGURATION_PROPERTY "mapping_configuration"

/**
* @brief The mapping configuration property node
* Use this to set the mapping configuration with a single string.
*/
#define FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION FEP3_DATA_REGISTRY_CONFIG "/" FEP3_MAPPING_CONFIGURATION_PROPERTY

/**
* @brief The mapping configuration file path property name
* Use this to set the mapping configuration with a file path from inside the data registry configuration node.
*/
#define FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY "mapping_configuration_file_path"

/**
* @brief The mapping configuration file path property node
* Use this to set the mapping configuration with a file path.
*/
#define FEP3_DATA_REGISTRY_MAPPING_CONFIGURATION_FILE_PATH FEP3_DATA_REGISTRY_CONFIG "/" FEP3_MAPPING_CONFIGURATION_FILE_PATH_PROPERTY

/**
* @brief The input signal renaming configuration property name.
* Use this to set the input signal renaming configuration with a single string from inside the data registry configuration node.
*/
#define FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY "renaming_input"

/**
* @brief The input signal renaming configuration property node.
* Use this to set the input signal renaming configuration with a single string.
*/
#define FEP3_DATA_REGISTRY_SIGNAL_RENAMING_INPUT_CONFIGURATION FEP3_DATA_REGISTRY_CONFIG "/" FEP3_SIGNAL_RENAMING_INPUT_CONFIGURATION_PROPERTY

/**
* @brief The output signal renaming configuration property name.
* Use this to set the output signal renaming configuration with a single string from inside the data registry configuration node.
*/
#define FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY "renaming_output"

/**
* @brief The output signal renaming configuration property node.
* Use this to set the output signal renaming configuration with a single string.
*/
#define FEP3_DATA_REGISTRY_SIGNAL_RENAMING_OUTPUT_CONFIGURATION FEP3_DATA_REGISTRY_CONFIG "/" FEP3_SIGNAL_RENAMING_OUTPUT_CONFIGURATION_PROPERTY


namespace fep3
{
namespace arya
{
    /**
     * @brief Interface for the data registry
     */
    class IDataRegistry
    {
    public:
        /// Definiton of the component interface identifier for the data registry
        FEP_COMPONENT_IID("data_registry.arya.fep3.iid");

    protected:
        /**
         * @brief DTOR
         * @note This DTOR is explicitly protected to prevent destruction via this interface.
         */
        ~IDataRegistry() = default;

    public:
        /// DataReceiver class provides an callbackentry for the @ref fep3::arya::IDataRegistry::registerDataReceiveListener function
        /// to receive data as a synchronous call (data triggered)
        using IDataReceiver = arya::ISimulationBus::IDataReceiver;

        /**
        * @brief Class providing access to input data
        *
        * @see fep3::IDataRegistry::getReader
        */
        class IDataReader
        {
        public:
            /// DTOR
            virtual ~IDataReader() = default;

            /**
            * @brief Gets the current size of the item queue
            *
            * @return The current size of the item queue
            */
            virtual size_t size() const = 0;
            /**
            * @brief Gets the current capacity of the item queue
            *
            * @return The current capacity of the item queue
            */
            virtual size_t capacity() const = 0;
            /**
            * @brief Pops the front item from the reader queue (if not empty) and passes it to the
            * callback of @p receiver. This method is non-blocking.
            * @remark If "data triggered" reception is currently running (see method @ref fep3::IDataRegistry::registerDataReceiveListener)
            * the reader queue is always empty because incoming data will immediately be passed to the "data triggered" receivers.
            *
            * @param[in] receiver The receiver object to be called back if an item is in the reader queue
            * @return ERR_NOERROR if succeded, error code otherwise:
            * @retval ERR_NOT_INITIALISED Data registry has not been initialized
            * @retval ERR_FAILED          No item has been passed to the receiver
            */
            virtual fep3::Result pop(IDataReceiver& receiver) = 0;
            /**
            * @brief Gets the time of the front item in the reader queue
            *
            * @return The timestamp of the front item, if at least one item is in the reader queue,
            *         invalid timestamp otherwise
            */
            virtual arya::Optional<arya::Timestamp> getFrontTime() const = 0;
        };

        /**
        * @brief Class providing access to output data
        * @see fep3::IDataRegistry::getWriter
        */
        class IDataWriter
        {
        public:
            /// DTOR
            virtual ~IDataWriter() = default;

            /**
            * @brief forwards the content of the @p data_sample to the simulation bus
            *
            * @param[in] data_sample The data sample to copy the content from
            * @return ERR_NOERROR if succeded, error code otherwise:
            * @retval ERR_NOT_INITIALISED Data registry has not been initialized
            * @retval ERR_UNEXPECTED      An unexpected error occurred.
            * @retval ERR_MEMORY          The transmit buffer's memory is not suitable to hold the @p data_sample's content.
            * @retval ERR_NOT_IMPL        There is no functioning implementation of this method.
            * @retval ERR_NOT_CONNECTED   The data writer is not connected to the simulation bus.
            *
            */
            virtual fep3::Result write(const arya::IDataSample& data_sample) = 0;
            /**
             * @brief forwards the content of the @p stream_type to the simulation bus
             *
             * @param[in] stream_type The stream type to copy the content from
             * @return fep3::Result ERR_NOERROR if succeded, error code otherwise (@see fep3::IDataRegistry::IDataWriter::write(const IDataSample&))
             */
            virtual fep3::Result write(const arya::IStreamType& stream_type) = 0;
            /**
             * @brief This method blocks until all content of the writer is forwarded.
             * Must be called after the last sample has been written by a job in a simulation cycle.
             *
             * @return fep3::Result ERR_NOERROR if succeded, error code otherwise:
             * @retval ERR_NOT_INITIALISED Data registry has not been initialized
             * @retval ERR_UNEXPECTED      An unexpected error occurred.
             * @retval ERR_NOT_CONNECTED   The data writer is not connected to a simulationbus resource.
             * @retval ERR_FAILED          The flushing failed.
             */
            virtual fep3::Result flush() = 0;
        };

        /**
         * @brief Registers an incoming date with the given @p name to the simulation bus when the participant is done initializing.
         * This implementation will *NOT* immediately register it to the simulation bus synchronously within this call.
         *
         * @param[in] name The name of the incoming data (must be unique and contain alphanumeric characters or underscore only)
         * @param[in] type The stream type of this data (see @ref fep3::arya::IStreamType)
         * @param[in] is_dynamic_meta_type The stream_meta_type may change of this data while streaming,
         *                                  this means the given type is the first expected one.
         * @return fep3::Result ERR_NOERROR if succeeded, error code otherwise:
         * @retval ERR_INVALID_TYPE  The name already exists with a different type
         * @retval ERR_NOT_SUPPORTED Stream type is not supported
         * @see unregisterDataIn, getReader, registerDataReceiveListener
         */
        virtual fep3::Result registerDataIn(const std::string& name,
                                            const arya::IStreamType& type,
                                            bool is_dynamic_meta_type=false) = 0;
        /**
         * @brief Registers an outgoing date with the given @p name to the simulation bus when the participant is done initializing.
         * This implementation will *NOT* immediately register it to the simulation bus synchronously within this call.
         *
         * @param[in] name The name of the outgoing data (must be unique and contain alphanumeric characters or underscore only)
         * @param[in] type The stream type of this data (see @ref fep3::arya::IStreamType)
         * @param[in] is_dynamic_meta_type The change of the meta type is allowed while sending.
         * @return fep3::Result ERR_NOERROR if succeeded, error code otherwise:
         * @retval ERR_INVALID_TYPE  The name already exists with a different type
         * @retval ERR_NOT_SUPPORTED Stream type is not supported
         * @see unregisterDataOut, getWriter
         */
        virtual fep3::Result registerDataOut(const std::string& name,
                                             const arya::IStreamType& type,
                                             bool is_dynamic_meta_type=false) = 0;
        /**
         * @brief Unregisters incoming data
         *
         * @param[in] name The name of the incoming data (must be unique)
         * @return fep3::Result ERR_NOERROR if succeeded, error code otherwise:
         * @retval ERR_NOT_FOUND No incoming data with this name found
         * @see registerDataIn
         */
        virtual fep3::Result unregisterDataIn(const std::string& name) = 0;

        /**
         * @brief Unregisters outgoing data
         *
         * @param[in] name The name of the outgoing data (must be unique)
         * @return fep3::Result ERR_NOERROR if succeeded, error code otherwise:
         * @retval ERR_NOT_FOUND No outgoing data with this name found
         * @see registerDataOut
         */
        virtual fep3::Result unregisterDataOut(const std::string& name) = 0;
        /**
         * @brief Registers a listener for data receive events and changes
         *
         * @param[in] name Name of the incoming data to listen to
         * @param[in] listener A shared pointer to the listener implementation
         * @return fep3::Result ERR_NOERROR if registration succeeded, error code otherwise:
         * @retval ERR_NOT_FOUND No incoming data with this name found
         * @see registerDataIn
         */
        virtual fep3::Result registerDataReceiveListener(const std::string& name,
            const std::shared_ptr<IDataReceiver>& listener) = 0;
        /**
         * @brief Unregisters a data receive listener
         *
         * @param[in] name Name of the incoming data to listen to
         * @param[in] listener A shared pointer to the listener implementation
         * @return fep3::Result ERR_NOERROR if unregisteration succeeded, error code otherwise:
         * @retval ERR_NOT_FOUND No incoming data with this name found
         */
        virtual fep3::Result unregisterDataReceiveListener(const std::string& name,
            const std::shared_ptr<IDataReceiver>& listener) = 0;

        /**
         * @brief Get a reader for the incoming data with the given @p name. Queue capacity is 1, so only the last item will be read.
         * @note Since incoming data and their readers get registered at the simulation bus only after the initialization
         *       of the participant is done this method has to be called during initialization or before.
         *
         * @param[in] name Name of the incoming data
         * @exception std::runtime_error Bad memory allocation
         * @return std::unique_ptr<IDataReader> The return pointer is only a nullptr if the signal is not registered with
         *  DataRegistry::addDataIn beforehand and the reader methods will return ERR_NOT_INITIALISED if the module is not at least in
         *  state FS_READY or the incoming data has been unregistered.
         */
        virtual std::unique_ptr<IDataRegistry::IDataReader> getReader(const std::string& name) = 0;
        /**
         * @brief Get a reader for the incoming data with the given @p name.
         * @note Since incoming data and their readers get registered at the simulation bus only after the initialization
         *       of the participant is done this method has to be called during initialization or before.
         *
         * @param[in] name Name of the incoming data
         * @param[in] queue_capacity The maximum number of items that the reader queue can hold at a time
         * @exception std::runtime_error Bad memory allocation
         * @return std::unique_ptr<IDataReader> The return pointer is only a nullptr if the signal is not registered with
         *  DataRegistry::addDataIn beforehand and the reader methods will return ERR_NOT_INITIALISED if the module is not at least in
         *  state FS_READY or the incoming data has been unregistered.
         */
        virtual std::unique_ptr<IDataRegistry::IDataReader> getReader(const std::string& name,
            size_t queue_capacity) = 0;
        /**
         * @brief Get a writer for the outgoing data with the given @p name. Queue capacity is 0, so data will be written immediately.
         * @note Since outgoing data and their writers get registered at the simulation bus only after the initialization
         *       of the participant is done this method has to be called during initialization or before.
         *
         * @param[in] name Name of the outgoing data
         * @exception std::runtime_error Bad memory allocation
         * @return std::unique_ptr<IDataWriter> The return pointer is always valid but the writer methods will return
         *  ERR_NOT_INITIALISED if the module is not at least in state FS_READY or the outgoing data has been unregistered.
         * @see IDataWriter, IDataWriter::write
         */
        virtual std::unique_ptr<IDataRegistry::IDataWriter> getWriter(const std::string& name) = 0;
        /**
         * @brief Get a writer for the outgoing data with the given @p name.
         * If queue_capacity is not 0 the writer will request a specific amount of sample buffer from the simulation bus.
         * @note Since outgoing data and their writers get registered at the simulation bus only after the initialization
         *       of the participant is done this method has to be called during initialization or before.
         *
         * @param[in] name Name of the outgoing data
         * @param[in] queue_capacity The maximum number of items that the transmit queue can hold at a time.
         * @exception std::runtime_error Bad memory allocation
         * @return std::unique_ptr<IDataWriter> The return pointer is always valid but the writer methods will return
         *  ERR_NOT_INITIALISED if the module is not at least in state FS_READY or the outgoing data has been unregistered.
         * @see IDataWriter, IDataWriter::write
         */
        virtual std::unique_ptr<IDataRegistry::IDataWriter> getWriter(const std::string& name,
            size_t queue_capacity) = 0;
    };

    /**
     * @deprecated
     * @brief Helper function to register data to a given registry and create a reader immediately.
     *
     * @param[in] data_registry The data registry to register the incoming data to
     * @param[in] name The name of the incoming data (must be unique and contain alphanumeric characters or underscore only)
     * @param[in] stream_type The stream type of this data (see @ref fep3::arya::IStreamType)
     * @param[in] queue_capacity The maximum number of items that the reader queue can hold at a time
     * @return std::unique_ptr<IDataRegistry::IDataReader> The return pointer is always valid but the reader methods will return
     *  ERR_NOT_INITIALISED if the module is not at least in state FS_READY or the incoming data has been unregistered.
     */
    inline std::unique_ptr<IDataRegistry::IDataReader> addDataIn(arya::IDataRegistry& data_registry,
        const std::string& name,
        const arya::IStreamType& stream_type,
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

    /**
     * @deprecated
     * @brief Helper function to register data to a given registry and create a writer immediately.
     *
     * @param[in] data_registry The data registry to register the outgoing data to
     * @param[in] name The name of the outgoing data (must be unique and contain alphanumeric characters or underscore only)
     * @param[in] stream_type The stream type of this data (see @ref fep3::arya::IStreamType)
     * @param[in] queue_capacity The maximum number of items that the transmit queue can hold at a time.
     * @return std::unique_ptr<IDataRegistry::IDataWriter> The return pointer is always valid but the writer methods will return
     *  ERR_NOT_INITIALISED if the module is not at least in state FS_READY or the outgoing data has been unregistered.
     */
    inline std::unique_ptr<arya::IDataRegistry::IDataWriter> addDataOut(arya::IDataRegistry& data_registry,
        const std::string& name,
        const arya::IStreamType& stream_type,
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

} // namespace arya
using arya::IDataRegistry;
} // namespace fep3
