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

#include <fep3/base/sample/data_sample_intf.h>
#include <fep3/base/stream_type/stream_type.h>
#include <fep3/components/base/component_iid.h>
#include <fep3/fep3_errors.h>
#include <fep3/fep3_optional.h>

#include <functional>

/**
 * @brief The Connext RTI DDS simulation bus main property tree entry node
 */
#define FEP3_RTI_DDS_SIMBUS_CONFIG "rti_dds_simulation_bus"

/**
 * @brief The participant domain property name
 * Use this to set the the participant domain by configuration.
 */
#define FEP3_SIMBUS_PARTICIPANT_DOMAIN_PROPERTY "participant_domain"

/**
 * @brief The participant domain configuration node
 * Use this to set the the participant domain by configuration.
 */
#define FEP3_RTI_DDS_SIMBUS_PARTICIPANT_DOMAIN                                                     \
    FEP3_RTI_DDS_SIMBUS_CONFIG "/" FEP3_SIMBUS_PARTICIPANT_DOMAIN_PROPERTY

/**
 * @brief Default value of participant domain property.
 */
#define FEP3_SIMBUS_PARTICIPANT_DOMAIN_DEFAULT_VALUE 5

/**
 * @brief The datawriter timeout configuration property name
 * Use this to set the timeout value how long a reader should wait for a writer to connect in
 * nanoseconds. Default value of 0 disables this feature.
 */
#define FEP3_SIMBUS_DATAWRITER_READY_TIMEOUT_PROPERTY "datawriter_ready_timeout"

/**
 * @brief The datawriter timeout configuration configuration node
 * Use this to set the timeout value how long a reader should wait for a writer to connect in
 * nanoseconds. Default value of 0 disables this feature.
 */
#define FEP3_RTI_DDS_SIMBUS_DATAWRITER_READY_TIMEOUT                                               \
    FEP3_RTI_DDS_SIMBUS_CONFIG "/" FEP3_SIMBUS_DATAWRITER_READY_TIMEOUT_PROPERTY

/**
 * @brief Default value of the datawriter timeout property in nanoseconds.
 */
#define FEP3_SIMBUS_DATAWRITER_READY_TIMEOUT_DEFAULT_VALUE 0

/**
 * @brief The semicolon-separated must be ready signal list configuration property name
 * Use this to set the set of signals for which the reader should wait for at least one writer to
 * connect.
 */
#define FEP3_SIMBUS_MUST_BE_READY_SIGNALS_PROPERTY "must_be_ready_signals"

/**
 * @brief The semicolon-separated must be ready signal list configuration configuration node
 * Use this to set the set of signals for which the reader should wait for at least one writer to
 * connect.
 */
#define FEP3_RTI_DDS_SIMBUS_MUST_BE_READY_SIGNALS                                                  \
    FEP3_RTI_DDS_SIMBUS_CONFIG "/" FEP3_SIMBUS_MUST_BE_READY_SIGNALS_PROPERTY

/**
 * @brief Default value of the "must be ready" signal list property (empty list).
 */
#define FEP3_SIMBUS_MUST_BE_READY_SIGNALS_DEFAULT_VALUE

/**
 * @brief The AsyncWaitSet of RTI DDS simulation bus configuration property name
 */
#define FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_PROPERTY "use_async_waitset"

/**
 * @brief The AsyncWaitSet of RTI DDS simulation bus configuration node
 * Use this to enable the AsyncWaitSet on RTI DDS simulation bus
 * Default value of false disables AsyncWaitSet
 */
#define FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET                                                          \
    FEP3_RTI_DDS_SIMBUS_CONFIG "/" FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_PROPERTY

/**
 * @brief Default value of the "use_async_waitset" property
 */
#define FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_DEFAULT_VALUE false

/**
 * @brief The AsyncWaitSet thread pool configuration node name
 */
#define FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_THREADS_PROPERTY "async_waitset_threads"

/**
 * @brief The AsyncWaitSet thread pool configuration node
 * Use this to set the size of this thread pool while using AsyncWaitSet
 * Default value of 8 opens 8 threads for async event handling
 */
#define FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_THREADS                                                  \
    FEP3_RTI_DDS_SIMBUS_CONFIG "/" FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_THREADS_PROPERTY

/**
 * @brief Default value of the "async_waitset_threads"
 */
#define FEP3_RTI_DDS_SIMBUS_ASYNC_WAITSET_THREADS_DEFAULT_VALUE 8

namespace fep3 {
namespace arya {

/**
 * @brief Interface for the simulation bus
 *
 * @attention Implementations of this class and subclasses are not necessarily thread safe.
 *            Thus, when using this interface, make sure each object is called from a single thread
 *            at a time only. There might be some exceptions from this rule explicitly described in
 *            the interface methods' documentation.
 */
class ISimulationBus {
public:
    // Definition of the component interface identifier for the simulation bus
    FEP_COMPONENT_IID("simulation_bus.arya.fep3.iid")

protected:
    /**
     * @brief DTOR
     * @note This DTOR is explicitly protected to prevent destruction via this interface.
     */
    ~ISimulationBus() = default;

public:
    /**
     * @brief Checks if the @p stream_type is supported by the simulation bus implementation
     *
     * @param[in] stream_type The stream type to be checked
     * @return @c true if the @p stream_type is supported by the simulation bus implementation, @c
     * false otherwise
     */
    virtual bool isSupported(const arya::IStreamType& stream_type) const = 0;

    class IDataReceiver;
    /**
     * @brief Class providing access to input data
     * @see fep3::ISimulationBus::getReader
     */
    class IDataReader {
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
         * @remark If "data triggered" reception is currently running (see methods @ref reset, @ref
         * ISimulationBus::startBlockingReception and @ref ISimulationBus::stopBlockingReception)
         * the reader queue is always empty because incoming data will immediately be passed to the
         * "data triggered" receivers.
         *
         * @param[in] receiver The receiver object to be called back if an item is in the reader
         * queue
         * @return @c true if an item has been popped, @c false otherwise
         */
        virtual bool pop(arya::ISimulationBus::IDataReceiver& receiver) = 0;

        /**
         * @brief Resets the receiver for "data triggered" data reception of this data reader to the
         *        passed @p receiver.
         * When @ref ISimulationBus::startBlockingReception is called, all incoming items will be
         * passed to the @p receiver.
         * @note The receiver will be called from within the thread context @ref
         *       ISimulationBus::startBlockingReception is called from.
         *
         * @param[in] receiver Shared pointer to the receiver to pass all incoming items to. If
         *                     empty "data triggered" reception is deactivated for this reader,
         *                     otherwise the reader takes ownership of the @p receiver and all
         *                     incoming items for this data reader will be reported to the
         *                     @p receiver when @ref ISimulationBus::startBlockingReception is
         *                     called.
         */
        virtual void reset(
            const std::shared_ptr<arya::ISimulationBus::IDataReceiver>& receiver = {}) = 0;

        /**
         * @brief Gets the time of the front item in the reader queue
         *
         * @return The timestamp of the front item, if at least one item is in the reader queue,
         *         empty optional otherwise
         */
        virtual arya::Optional<arya::Timestamp> getFrontTime() const = 0;
    };

    /**
     * @brief Class for receiving data and stream types
     */
    class IDataReceiver {
    public:
        /// DTOR
        virtual ~IDataReceiver() = default;

        /**
         * @brief Receives a stream type item
         *
         * @param[in] type The received stream type
         */
        virtual void operator()(const arya::data_read_ptr<const arya::IStreamType>& type) = 0;

        /**
         * @brief Receives a data sample item
         *
         * @param[in] sample The received data sample
         */
        virtual void operator()(const arya::data_read_ptr<const arya::IDataSample>& sample) = 0;
    };

    /**
     * @brief Class providing data transmission facilities
     * @see fep3::ISimulationBus::getWriter
     */
    class IDataWriter {
    public:
        /// DTOR
        virtual ~IDataWriter() = default;

        /**
         * @brief Copies the content of the @p data_sample into the transmit buffer
         *
         * @param[in] data_sample The data sample to copy the content from
         * @return ERR_NOERROR if succeeded, error code otherwise:
         * @retval ERR_UNEXPECTED An unexpected error occurred.
         * @retval ERR_MEMORY The transmit buffer's memory is not suitable to hold the @p
         * data_sample's content.
         * @retval ERR_NOT_IMPL There is no functioning implementation of this method.
         * @retval ERR_NOT_CONNECTED The data writer is not connected to a transmit buffer.
         */
        virtual fep3::Result write(const arya::IDataSample& data_sample) = 0;

        /**
         * @brief This is an overloaded member function that copies the content of the @p
         * stream_type into the transmit buffer
         *
         * @param[in] stream_type T stream type to copy the content from
         * @return fep3::Result ERR_NOERROR if succeeded, error code otherwise (see above)
         */
        virtual fep3::Result write(const arya::IStreamType& stream_type) = 0;

        /**
         * @brief Transmits the content of the transmit buffer. This method blocks until all content
         * of the transmit buffer has been transmitted.
         *
         * @return fep3::Result ERR_NOERROR if succeeded, error code otherwise:
         * @retval ERR_UNEXPECTED An unexpected error occurred.
         * @retval ERR_NOT_IMPL There is no functioning implementation of this method.
         * @retval ERR_NOT_CONNECTED The data writer is not connected to a transmission resource.
         * @retval ERR_FAILED The transmission failed.
         */
        virtual fep3::Result transmit() = 0;
    };

    /**
     * @brief Gets a reader for data on an input signal of the given static @p stream_type with the
     * given signal @p name whose queue capacity is 1.
     * @note The reader pre-allocates a sample pool of size 1 for storing incoming samples.
     *       Holding strong references to the samples as delivered to the receiver of
     *       @ref fep3::arya::ISimulationBus::IDataReader::pop
     *       or @ref fep3::arya::ISimulationBus::IDataReader::reset
     *       causes the pool items to be unavailable for further incoming samples. If
     *       pool items are still unavailable when samples come in, additional storage will
     *       be allocated on the heap, which might lead to system calls and therefore renders the
     *       FEP Participant unsuitable for real time environments.
     *
     * @param[in] name The name of the input signal (must be unique)
     * @param[in] stream_type The stream type of the input signal (see @ref fep3::arya::IStreamType)
     * @return Data reader if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal @p name); unregistration is automatically
     *         performed when the data reader is destroyed
     */
    virtual std::unique_ptr<arya::ISimulationBus::IDataReader> getReader(
        const std::string& name, const arya::IStreamType& stream_type) = 0;
    /**
     * This is an overloaded member function that gets a reader for data on an input
     * signal with the given signal @p name whose queue capacity is @p queue_capacity. The queue
     * behaves like a FIFO: If the queue is full the oldest sample (according to the samples'
     * timestamp) will be discarded upon arrival of a new sample.
     * @note The reader pre-allocates a sample pool of @p queue_capacity for storing incoming
     * samples. Holding strong references to the samples as delivered to the receiver of
     *       @ref fep3::arya::ISimulationBus::IDataReader::pop
     *       or @ref fep3::arya::ISimulationBus::IDataReader::reset
     *       causes the pool items to be unavailable for further incoming samples. If
     *       pool items are still unavailable when samples come in, additional storage will
     *       be allocated on the heap, which might lead to system calls and therefore renders the
     *       FEP Participant unsuitable for real time environments.
     *
     * @param[in] name The name of the input signal (must be unique)
     * @param[in] stream_type The stream type of the input signal (see @ref fep3::arya::IStreamType)
     * @param[in] queue_capacity The capacity of the queue.
     * @return Data reader if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal @p name); unregistration is automatically
     *         performed when the data reader is destroyed
     */
    virtual std::unique_ptr<arya::ISimulationBus::IDataReader> getReader(
        const std::string& name, const arya::IStreamType& stream_type, size_t queue_capacity) = 0;

    /**
     * @brief This is an overloaded member function that gets a reader for data on an input
     * signal of dynamic stream type with the given signal @p name.
     * @note Calling this member function renders the FEP Participant unsuitable for real time
     *       environments, because a dynamic change of the stream type may require dynamic memory
     *       allocation.
     *
     * @param[in] name The name of the input signal (must be unique)
     * @return Data reader if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal @p name); unregistration is automatically
     *         performed when the data reader is destroyed
     */
    virtual std::unique_ptr<arya::ISimulationBus::IDataReader> getReader(
        const std::string& name) = 0;

    /**
     * This is an overloaded member function that gets a reader for data on an input
     * signal of dynamic stream type with the given signal @p name whose queue capacity is @p
     * queue_capacity. The queue behaves like a FIFO: If the queue is full the oldest sample
     * (according to the samples' timestamp) will be discarded upon arrival of a new sample.
     * @note Calling this member function renders the FEP Participant unsuitable for real time
     *       environments, because a dynamic change of the stream type may require dynamic memory
     *       allocation.
     *
     * @param[in] name The name of the input signal (must be unique)
     * @param[in] queue_capacity The capacity of the queue.
     * @return Data reader if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal @p name); unregistration is automatically
     *         performed when the data reader is destroyed
     */
    virtual std::unique_ptr<arya::ISimulationBus::IDataReader> getReader(const std::string& name,
                                                                         size_t queue_capacity) = 0;

    /**
     * @brief Gets a writer for data on an output signal of the given static @p stream_type with
     * the given signal @p name.
     *
     * @param[in] name The name of the output signal (must be unique)
     * @param[in] stream_type The stream type of the output signal (see @ref
     * fep3::arya::IStreamType)
     * @return Data writer if succeeded, nullptr otherwise  (e. g. if this method has already been
     *         called for the signal with the given signal @p name)
     */
    virtual std::unique_ptr<arya::ISimulationBus::IDataWriter> getWriter(
        const std::string& name, const arya::IStreamType& stream_type) = 0;

    /**
     * This is an overloaded member function that gets a writer for data on an output signal
     * of the given static @p stream_type with the given signal @p name whose queue capacity is @p
     * queue_capacity. The queue behaves like a FIFO: If the queue is full the oldest sample
     * (according to the samples' timestamp) will be discarded upon arrival of a new sample.
     *
     * @param[in] name The name of the output signal (must be unique)
     * @param[in] stream_type The stream type of the output signal (see @ref
     * fep3::arya::IStreamType)
     * @param[in] queue_capacity The capacity of the queue.
     * @return Data writer if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal @p name)
     */
    virtual std::unique_ptr<arya::ISimulationBus::IDataWriter> getWriter(
        const std::string& name, const arya::IStreamType& stream_type, size_t queue_capacity) = 0;

    /**
     * @brief This is an overloaded member function that gets a writer for data on an output
     * signal of dynamic stream type with the given signal @p name.
     *
     * @param[in] name The name of the output signal (must be unique)
     * @return Data writer if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal @p name)
     */
    virtual std::unique_ptr<arya::ISimulationBus::IDataWriter> getWriter(
        const std::string& name) = 0;

    /**
     * @brief This is an overloaded member function that gets a writer for data on an output
     * signal of dynamic stream type with the given signal @p name whose queue capacity is @p
     * queue_capacity. The queue behaves like a FIFO: If the queue is full the oldest sample
     * (according to the samples' timestamp) will be discarded upon arrival of a new sample.
     *
     * @param[in] name The name of the output signal (must be unique)
     * @param[in] queue_capacity The capacity of the queue.
     * @return Data writer if succeeded, nullptr otherwise (e. g. if this method has already been
     *         called for the signal with the given signal @p name)
     */
    virtual std::unique_ptr<arya::ISimulationBus::IDataWriter> getWriter(const std::string& name,
                                                                         size_t queue_capacity) = 0;

    /**
     * @brief Starts passing all incoming items to the receivers of the data readers.
     * @see IDataReader::reset
     * This method registers for reception of incoming data at the transmission layer,
     * prepares for a call to @ref stopBlockingReception and then calls @p
     * reception_preparation_done_callback. If at least one existing reader has a receiver set via
     * @ref IDataReader::reset, this method blocks until @ref stopBlockingReception is called.
     * Otherwise this method does not block. Use this method to implement "data triggered" behavior.
     * This method is thread-safe against calls to @ref stopBlockingReception.
     * @note The receivers of the data readers will be called from within the thread context this
     * method is called from.
     * @note To ensure the Simulation Bus is fully prepared for data reception as well as for a call
     * to @ref stopBlockingReception, wait until @p reception_preparation_done_callback has been
     * called before calling any other method on the Simulation Bus.
     *
     * @param reception_preparation_done_callback Callback to be called once the reception is
     * prepared
     */
    virtual void startBlockingReception(
        const std::function<void()>& reception_preparation_done_callback) = 0;

    /**
     * @brief Stops all receptions running due to calls of method @ref startBlockingReception. This
     * causes all blocking calls of method @ref startBlockingReception to return. If there are
     * currently no blocking calls to @ref startBlockingReception, this method does nothing. This
     * method is thread-safe against calls to @ref startBlockingReception.
     */
    virtual void stopBlockingReception() = 0;
};

} // namespace arya
using arya::ISimulationBus;
} // namespace fep3
