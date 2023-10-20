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

#include <fep3/base/sample/data_sample.h>
#include <fep3/base/stream_type/default_stream_type.h>

#include <random>

using namespace fep3;

/**
 * @brief Data sample helper template to wrap a non standard layout type T and support time and
 * counter
 * @tparam T the non standard layout type
 */
template <typename T>
class TimeDataSampleType : public IDataSample, public base::RawMemoryStandardType<T> {
public:
    /// value type of DataSampleType
    typedef T ValueType;
    /// super type of DataSampleType
    typedef base::RawMemoryStandardType<T> BaseType;

public:
    /**
     * CTOR
     * @param value reference to the value type
     */
    TimeDataSampleType(ValueType& value) : BaseType(value)
    {
    }

    /**
     * CTOR
     * @param value reference to the value type
     * @param time initial timestamp
     */
    TimeDataSampleType(ValueType& value, const fep3::arya::Timestamp& time)
        : BaseType(value), _timestamp(time)
    {
    }

    /**
     * copy operator
     * @param other reference to the value type to copy from
     * @return this data sample type object
     */
    TimeDataSampleType& operator=(const TimeDataSampleType& other)
    {
        other.read(*this);
        return *this;
    }

public:
    Timestamp getTime() const override
    {
        return _timestamp;
    }
    size_t getSize() const override
    {
        return BaseType::size();
    }
    uint32_t getCounter() const override
    {
        return _counter;
    }
    size_t read(IRawMemory& writeable_memory) const override
    {
        return writeable_memory.set(BaseType::cdata(), BaseType::size());
    }

    void setTime(const Timestamp& time) override
    {
        _timestamp = time;
    }

    size_t write(const IRawMemory& from_memory) override
    {
        return BaseType::set(from_memory.cdata(), from_memory.size());
    }
    void setCounter(uint32_t counter) override
    {
        _counter = counter;
    }

private:
    fep3::arya::Timestamp _timestamp;
    uint32_t _counter = 0;
};

class RandomSample : public base::DataSample {
private:
    size_t _size;

public:
    RandomSample(size_t size) : DataSample(size, true), _size(size)
    {
        fillRandom();
    }

    void fillRandom()
    {
        std::vector<uint8_t> data(_size);

        std::random_device rd;
        std::mt19937 mt(rd());
        // standard does not allow uint8_t distribution
        std::uniform_int_distribution<uint16_t> dist(
            0, static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()));
        // we are sure that the random number is inside the numberical limits of uint8_t
        std::generate(data.begin(), data.end(), [&]() { return static_cast<uint8_t>(dist(mt)); });
        set(data.data(), data.size());
    }

    bool compare(const base::DataSample& sample) const
    {
        if (getSize() != sample.getSize()) {
            return false;
        }

        auto data = reinterpret_cast<const uint8_t*>(sample.cdata());
        auto own_data = reinterpret_cast<const uint8_t*>(cdata());
        return std::equal(data, std::next(data, sample.getSize()), own_data);
    }
};

class RawDataSample : public IRawMemory {
public:
    RawDataSample()
    {
    }

public:
    size_t capacity() const
    {
        return 1000;
    }

    const void* cdata() const
    {
        return _data.data();
    }

    size_t size() const
    {
        return _data.size();
    }

    size_t set(const void* data, size_t data_size)
    {
        _data = std::string(static_cast<const char*>(data), data_size);
        return data_size;
    }

    size_t resize(size_t data_size)
    {
        return data_size;
    }

    std::string getString()
    {
        return _data;
    }

private:
    std::string _data;
};
