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

#include <fep3/components/simulation_bus/simulation_bus_intf.h>

#include <gtest/gtest.h>

#include <condition_variable>

using namespace fep3;

#define NO_CHECK -1

/**
 * @brief Basic receiver collecting all stream_types and samples
 */
struct BaseBlockingTestReceiver : public ISimulationBus::IDataReceiver {
public:
    std::condition_variable _reception_condition;
    std::mutex _reception_mutex;

    virtual ~BaseBlockingTestReceiver() = default;

    virtual bool hasReceivedAllExpected() = 0;

    bool waitFor(std::chrono::seconds timeout = std::chrono::seconds(5))
    {
        std::unique_lock<std::mutex> reception_lock(_reception_mutex);
        return _reception_condition.wait_for(
            reception_lock, timeout, [this]() { return hasReceivedAllExpected(); });
    }
};

struct BlockingTestReceiver : public BaseBlockingTestReceiver {
public:
    BlockingTestReceiver(ISimulationBus& /*simulation_bus*/) : BaseBlockingTestReceiver()
    {
    }

    void operator()(const data_read_ptr<const IStreamType>& stream_type) override
    {
        _stream_types.push_back(stream_type);
        _reception_condition.notify_all();
    }
    void operator()(const data_read_ptr<const IDataSample>& sample) override
    {
        _samples.push_back(sample);
        _reception_condition.notify_all();
    }

    void waitFor(int32_t wait_for_samples,
                 int32_t wait_for_stream_types,
                 std::chrono::seconds timeout = std::chrono::seconds(5),
                 bool /*stop_receiving*/ = true,
                 bool /*expect_timeout*/ = false)
    {
        _wait_for_samples = wait_for_samples;
        _wait_for_stream_types = wait_for_stream_types;

        BaseBlockingTestReceiver::waitFor(timeout);

        if (_wait_for_samples != -1) {
            EXPECT_EQ(_samples.size(), _wait_for_samples);
        }

        if (_wait_for_stream_types != -1) {
            EXPECT_EQ(_stream_types.size(), _wait_for_stream_types);
        }
    }

    bool hasReceivedAllExpected() override
    {
        if (_wait_for_samples == NO_CHECK &&
            _wait_for_stream_types == NO_CHECK) { // if no check shall be done at all, at least one
                                                  // item must have been received
            if (_samples.size() > 0 || _stream_types.size() > 0) {
                return true;
            }
        }
        else if ((_wait_for_samples <= NO_CHECK ||
                  _samples.size() == static_cast<uint32_t>(_wait_for_samples)) &&
                 (_wait_for_stream_types <= NO_CHECK ||
                  _stream_types.size() == static_cast<uint32_t>(_wait_for_stream_types))) {
            return true;
        }
        return false;
    }

    void clear()
    {
        _samples.clear();
        _stream_types.clear();
    }

public:
    std::vector<data_read_ptr<const IStreamType>> _stream_types;
    std::vector<data_read_ptr<const IDataSample>> _samples;
    int32_t _wait_for_samples = 1;
    int32_t _wait_for_stream_types = 0;
};

class TestReceiver : public ISimulationBus::IDataReceiver {
public:
    std::vector<data_read_ptr<const IStreamType>> _stream_types;
    std::vector<data_read_ptr<const IDataSample>> _samples;

    int32_t _wait_for_samples = 1;
    int32_t _wait_for_streamtypes = 0;

    TestReceiver()
    {
    }

    virtual void operator()(const data_read_ptr<const IStreamType>& stream_type) override
    {
        _stream_types.push_back(stream_type);
    }

    virtual void operator()(const data_read_ptr<const IDataSample>& sample) override
    {
        _samples.push_back(sample);
    }

    void clear()
    {
        _samples.clear();
        _stream_types.clear();
    }
};

class OrderTestReceiver : public TestReceiver {
private:
    bool _last_stream_type = false;

public:
    virtual void operator()(const data_read_ptr<const IStreamType>& /*stream_type*/) override
    {
        EXPECT_FALSE(_last_stream_type);
        _last_stream_type = true;
    }
    virtual void operator()(const data_read_ptr<const IDataSample>& /*sample*/) override
    {
        EXPECT_TRUE(_last_stream_type);
        _last_stream_type = false;
    }
};

class CountSampleTestReceiver : public TestReceiver {
private:
    uint32_t _count_samples = 0;
    uint32_t _count_stream_type = 0;

public:
    virtual void operator()(const data_read_ptr<const IStreamType>& /*stream_type*/) override
    {
        _count_stream_type++;
    }
    virtual void operator()(const data_read_ptr<const IDataSample>& /*sample*/) override
    {
        _count_samples++;
    }

    uint32_t getSampleCount()
    {
        auto count_samples = _count_samples;
        _count_samples = 0;
        return count_samples;
    }

    uint32_t getStreamTypeCount()
    {
        auto count_stream_type = _count_stream_type;
        _count_stream_type = 0;
        return count_stream_type;
    }
};
