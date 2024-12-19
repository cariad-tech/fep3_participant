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
#include <fep3/base/sample/raw_memory.h>

#include <a_util/memory.h>

namespace fep3 {
namespace base {
namespace arya {

/**
 * Abstract base class for a data sample providing time and counter functionality
 */
class DataSampleBase : public fep3::arya::IDataSample {
public:
    /**
     * @brief Constructor
     * Constructs a data sample base
     *
     * @param[in] time Timestamp to be set
     * @param[in] counter Counter value to be set
     */
    DataSampleBase(fep3::arya::Timestamp time = fep3::arya::Timestamp(0), uint32_t counter = 0)
        : _time(time), _counter(counter)
    {
    }

protected:
    /// @brief Copy constructor
    DataSampleBase(const DataSampleBase&) = default;
    /// @brief Move constructor
    DataSampleBase(DataSampleBase&&) = default;
    /**
     * @brief Copy assignment
     *
     * @return Reference to this data sample
     */
    DataSampleBase& operator=(const DataSampleBase&) = default;
    /**
     * @brief Move assignment
     *
     * @return Reference to this data sample
     */
    DataSampleBase& operator=(DataSampleBase&&) = default;
    /**
     * Assigns another data sample to this
     * @param[in] other The data sample to take the timestamp and the counter from
     * @return Reference to this data sample
     */
    DataSampleBase& operator=(const fep3::arya::IDataSample& other)
    {
        setTime(other.getTime());
        setCounter(other.getCounter());
        return *this;
    }
    /**
     * @brief Constructor
     * Constructs a data sample base from another data sample
     *
     * @param[in] other The data sample to take the timestamp and the counter from
     */
    DataSampleBase(const fep3::arya::IDataSample& other)
        : _time(other.getTime()), _counter(other.getCounter())
    {
    }

public:
    /// @brief Destructor
    virtual ~DataSampleBase() = default;

    /**
     * @brief Gets the timestamp
     * @return The timestamp of this data sample
     */
    fep3::arya::Timestamp getTime() const override
    {
        return _time;
    }

    /**
     * @brief Gets the counter value
     * @return The counter value of this data sample
     */
    uint32_t getCounter() const override
    {
        return _counter;
    }

    /**
     * @brief Sets the timestamp
     * @param[in] time The timestamp to be set
     */
    void setTime(const fep3::arya::Timestamp& time) override
    {
        _time = time;
    }

    /**
     * @brief Sets the counter
     * @param[in] counter The counter value to be set
     */
    void setCounter(uint32_t counter) override
    {
        _counter = counter;
    }

private:
    fep3::arya::Timestamp _time;
    uint32_t _counter;
};

/**
 * Implementation helper class for a data sample used within ISimulationBus or IDataRegistry
 */
class DataSample : public arya::DataSampleBase, public fep3::arya::IRawMemory {
private:
    using super = arya::DataSampleBase;

public:
    /**
     * @brief Construct a new data sample
     */
    DataSample() : _fixed_size(false), _current_size(0)
    {
    }

    /**
     * @brief Construct a new data sample
     *
     * @param[in] pre_allocated_capacity will preallocate the raw memory data area of the data
     * sample
     * @param[in] fixed_size marks the sample to have no dynamic memory. if capacity size is reached
     * no reallocation is possible.
     */
    DataSample(size_t pre_allocated_capacity, bool fixed_size)
        : _fixed_size(fixed_size), _current_size(0), _buffer(pre_allocated_capacity)
    {
    }

    /**
     * @brief Construct a new data sample
     *
     * @param[in] time preset timestamp
     * @param[in] counter preset counter
     * @param[in] from_memory the memory to copy the content from
     */
    DataSample(fep3::arya::Timestamp time,
               uint32_t counter,
               const fep3::arya::IRawMemory& from_memory)
        : super(time, counter), _fixed_size(false)
    {
        write(from_memory);
    }

    /**
     * @brief copy construct a new data sample
     *
     * @param[in] other the sample to copy from
     */
    DataSample(const DataSample& other) : super(other), _fixed_size(false)
    {
        other.read(*this);
    }

    /**
     * @brief copy construct a new data sample
     *
     * @param[in] other the sample interface to copy from
     */
    DataSample(const fep3::arya::IDataSample& other) : super(other), _fixed_size(false)
    {
        other.read(*this);
    }

    /**
     * @brief move construct a new data sample
     *
     * @param[in,out] other the sample to move from
     */
    DataSample(DataSample&& other)
        : super(other),
          _fixed_size(other._fixed_size),
          _current_size(other._current_size),
          _buffer(std::move(other._buffer))
    {
    }

    /**
     * @brief copy the content of the given data sample
     *
     * @param[in] other the sample to copy from
     * @return DataSample& the copied sample content
     */
    DataSample& operator=(const DataSample& other)
    {
        super::operator=(other);
        other.read(*this);
        return *this;
    }

    /**
     * @brief copy the content of the given data sample interface
     *
     * @param[in] other the sample content to copy from
     * @return DataSample& the copied sample content
     */
    DataSample& operator=(const fep3::arya::IDataSample& other)
    {
        setTime(other.getTime());
        setCounter(other.getCounter());
        other.read(*this);
        return *this;
    }

    /**
     * @brief move the content of the given data sample
     *
     * @param[in,out] other the sample content to move from
     * @return DataSample& the copied sample content
     */
    DataSample& operator=(DataSample&& other)
    {
        super::operator=(other);
        std::swap(_fixed_size, other._fixed_size);
        std::swap(_current_size, other._current_size);
        std::swap(_buffer, other._buffer);
        return *this;
    }

public:
    /// @brief Destructor
    ~DataSample() override = default;

    /**
     * @brief set the content of the sample
     *
     * @param[in] time timestamp
     * @param[in] counter counter of the sample
     * @param[in] from_memory the memory to copy from
     * @return size_t the size copied
     */
    size_t update(const fep3::arya::Timestamp& time,
                  uint32_t counter,
                  const fep3::arya::IRawMemory& from_memory)
    {
        setTime(time);
        setCounter(counter);
        return write(from_memory);
    }

public:
    size_t capacity() const override
    {
        return _buffer.getSize();
    }

    const void* cdata() const override
    {
        return _buffer.getPtr();
    }

    size_t size() const override
    {
        return _current_size;
    }

    size_t set(const void* data, size_t data_size) override
    {
        if (_fixed_size && capacity() < data_size) {
            a_util::memory::copy(_buffer, data, capacity());
            _current_size = capacity();
        }
        else {
            a_util::memory::copy(_buffer, data, data_size);
            _current_size = data_size;
        }
        return _current_size;
    }
    size_t resize(size_t data_size) override
    {
        if (_fixed_size && capacity() < data_size) {
            _current_size = capacity();
        }
        else {
            _current_size = data_size;
        }
        return _current_size;
    }

public:
    size_t getSize() const override
    {
        return _current_size;
    }
    size_t read(fep3::arya::IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(cdata(), size());
    }
    size_t write(const fep3::arya::IRawMemory& from_memory) override
    {
        return set(from_memory.cdata(), from_memory.size());
    }

private:
    bool _fixed_size;
    size_t _current_size;
    a_util::memory::MemoryBuffer _buffer;
};

/**
 * @brief Data sample helper class to wrap a raw memory pointer @p data and a size @p data_size in
 * bytes.
 */
struct DataSampleRawMemoryRef : public fep3::arya::IDataSample {
    /**
     * CTOR
     * @param[in,out] time the timestamp to preset
     * @param[in] data the data pointer to reference to
     * @param[in] data_size the size of the memory given in @p data
     */
    explicit DataSampleRawMemoryRef(fep3::arya::Timestamp& time, const void* data, size_t data_size)
        : _time(time), _raw_memory_ref(data, data_size)
    {
    }

private:
    fep3::arya::Timestamp& _time;
    arya::RawMemoryRef _raw_memory_ref;

    fep3::arya::Timestamp getTime() const override
    {
        return _time;
    }
    size_t getSize() const override
    {
        return _raw_memory_ref.size();
    }
    uint32_t getCounter() const override
    {
        return 0;
    }

    size_t read(fep3::arya::IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(_raw_memory_ref.cdata(), _raw_memory_ref.size());
    }
    void setTime(const fep3::arya::Timestamp& time) override
    {
        _time = time;
    }
    size_t write(const fep3::arya::IRawMemory& /*from_memory*/) override
    {
        return 0;
    }
    void setCounter(uint32_t /*counter*/) override
    {
    }
};

/**
 * @brief Data sample helper template to wrap a non-standard layout type T by default
 * @tparam T the non-standard layout type
 * @tparam is_standard_layout_type the standard layout type check
 */
template <typename T, typename is_standard_layout_type = void>
class DataSampleType : public arya::DataSampleBase, public arya::RawMemoryClassType<T> {
private:
    using super = arya::DataSampleBase;

public:
    /// the value type is T
    typedef T value_type;
    /// the base type is the super class
    typedef arya::RawMemoryClassType<T> base_type;

    /**
     * CTOR
     * @param[in] other the other to copy from
     */
    explicit DataSampleType(value_type& other) : base_type(other)
    {
    }

protected:
    /**
     * @brief Deleted Copy CTOR
     */
    DataSampleType(const DataSampleType&) = delete;

    /**
     * @brief Deleted Move CTOR
     */
    DataSampleType(DataSampleType&&) = delete;

    /**
     * Copy assignment
     * @param[in] other the other to copy from
     * @return this type of sample
     */
    DataSampleType& operator=(const DataSampleType& other)
    {
        super::operator=(other);
        other.read(*this);
        return *this;
    }

    /**
     * @brief Deleted Move assignment operator
     *
     * @return DataSampleType&
     */
    DataSampleType& operator=(DataSampleType&&) = delete;

    /**
     * Assignment from other sample
     * @param[in] other the other to copy from
     * @return this type of sample
     */
    DataSampleType& operator=(const fep3::arya::IDataSample& other)
    {
        super::operator=(other);
        other.read(*this);
        return *this;
    }

public:
    /**
     * @brief Default DTOR
     */
    ~DataSampleType() = default;

    size_t getSize() const override
    {
        return base_type::size();
    }

    size_t read(fep3::arya::IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(base_type::cdata(), base_type::size());
    }

    size_t write(const fep3::arya::IRawMemory& from_memory) override
    {
        return base_type::set(from_memory.cdata(), from_memory.size());
    }
};

/// @cond nodoc
/**
 * @brief Data sample helper template to wrap a standard layout type T by default
 * @tparam T the standard layout type
 */
template <typename T>
class DataSampleType<T, typename std::enable_if<std::is_standard_layout<T>::value>::type>
    : public arya::DataSampleBase, public arya::RawMemoryStandardType<T> {
private:
    using super = arya::DataSampleBase;

public:
    /// value type of DataSampleType
    typedef T value_type;
    /// super type of DataSampleType
    typedef arya::RawMemoryStandardType<T> base_type;

    /**
     * CTOR
     * @param[in] value reference to the value type
     */
    explicit DataSampleType(value_type& value) : base_type(value)
    {
    }

protected:
    /**
     * @brief Deleted Copy CTOR
     */
    DataSampleType(const DataSampleType&) = delete;

    /**
     * @brief Deleted Move CTOR
     */
    DataSampleType(DataSampleType&&) = delete;

    /**
     * Copy assignment
     * @param[in] other reference to the value type to copy from
     * @return this data sample type object
     */
    DataSampleType& operator=(const DataSampleType& other)
    {
        super::operator=(other);
        other.read(*this);
        return *this;
    }

    /**
     * @brief Deleted Move assignment
     *
     * @return DataSampleType&
     */
    DataSampleType& operator=(DataSampleType&&) = delete;

    /**
     * Assignment from other sample
     * @param[in] other the other to copy from
     * @return this sample type
     */
    DataSampleType& operator=(const fep3::arya::IDataSample& other)
    {
        super::operator=(other);
        other.read(*this);
        return *this;
    }

public:
    /**
     * @brief Default DTOR
     */
    ~DataSampleType() = default;

    size_t getSize() const override
    {
        return base_type::size();
    }

    size_t read(fep3::arya::IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(base_type::cdata(), base_type::size());
    }

    size_t write(const fep3::arya::IRawMemory& from_memory) override
    {
        return base_type::set(from_memory.cdata(), from_memory.size());
    }
};
/// @endcond

/**
 * @brief Data sample helper template to wrap a std vector of type T
 */
template <typename T>
class StdVectorSampleType : public arya::RawMemoryClassType<std::vector<T>>,
                            public arya::DataSampleBase {
private:
    using super = DataSampleBase;

public:
    /// value type of DataSampleType
    typedef T value_type;
    /// super type of DataSampleType
    typedef arya::RawMemoryClassType<std::vector<T>> base_type;

    /**
     * CTOR
     * @param[in] array the data pointer to reference to
     */
    explicit StdVectorSampleType(std::vector<value_type>& array) : base_type(array)
    {
    }

protected:
    /**
     * @brief Deleted Copy CTOR
     */
    StdVectorSampleType(const StdVectorSampleType&) = delete;

    /**
     * @brief Deleted Move CTOR
     */
    StdVectorSampleType(StdVectorSampleType&&) = delete;

    /**
     * copy assignment
     * @param[in] other reference to the value type to copy from
     * @return this data sample type object
     */
    StdVectorSampleType& operator=(const fep3::arya::IDataSample& other)
    {
        super::operator=(other);
        other.read(*this);
        return *this;
    }

    /**
     * @brief Deleted Move assignment
     *
     * @return StdVectorSampleType&
     */
    StdVectorSampleType& operator=(StdVectorSampleType&&) = delete;

public:
    /**
     * @brief Default DTOR
     */
    ~StdVectorSampleType() = default;

    size_t getSize() const override
    {
        return base_type::size();
    }

    size_t read(fep3::arya::IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(base_type::cdata(), base_type::size());
    }

    size_t write(const fep3::arya::IRawMemory& from_memory) override
    {
        base_type::set(from_memory.cdata(), from_memory.size());
        return getSize();
    }
};

} // namespace arya
using arya::DataSample;
using arya::DataSampleRawMemoryRef;
using arya::DataSampleType;
using arya::StdVectorSampleType;
} // namespace base
} // namespace fep3
