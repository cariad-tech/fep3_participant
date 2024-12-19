/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "internal_topic.h"

#include <fep3/base/sample/data_sample.h>

using namespace fep3;
using namespace dds::domain;

InternalTopic::InternalTopic(const std::string& topic_name) : _topic_name(topic_name)
{
}

std::string InternalTopic::GetTopic()
{
    return _topic_name;
}

std::unique_ptr<fep3::ISimulationBus::IDataReader> InternalTopic::createDataReader(
    size_t, /*queue_capacity*/
    const std::weak_ptr<
        fep3::base::SimulationDataAccessCollection<ReaderItemQueue>>& /*data_access_collection*/
)
{
    return std::make_unique<InternalTopic::InternalReader>(shared_from_this());
}

std::unique_ptr<fep3::ISimulationBus::IDataWriter> InternalTopic::createDataWriter(
    size_t /*queue_capacity*/)
{
    return {};
}

void InternalTopic::write(const std::string& data)
{
    std::lock_guard<std::recursive_mutex> guard(_queue_mutex);
    while (_queue.size() > 10) {
        _queue.pop();
    }

    _queue.push(data);
}

InternalTopic::InternalReader::InternalReader(const std::shared_ptr<InternalTopic>& internal_topic)
    : _internal_topic(internal_topic)
{
}

size_t InternalTopic::InternalReader::size() const
{
    std::lock_guard<std::recursive_mutex> guard(_internal_topic->_queue_mutex);
    return _internal_topic->_queue.size();
}

size_t InternalTopic::InternalReader::capacity() const
{
    std::lock_guard<std::recursive_mutex> guard(_internal_topic->_queue_mutex);
    return _internal_topic->_queue.size() + 10;
}

bool InternalTopic::InternalReader::pop(fep3::ISimulationBus::IDataReceiver& receiver)
{
    if (size() > 0) {
        std::lock_guard<std::recursive_mutex> guard(_internal_topic->_queue_mutex);
        const std::string data = _internal_topic->_queue.front();
        _internal_topic->_queue.pop();

        auto sample = std::make_shared<base::DataSample>();
        sample->set(data.data(), data.size());
        sample->setTime(std::chrono::seconds(0));
        sample->setCounter(0);

        receiver(sample);
        return true;
    }
    return false;
}

void InternalTopic::InternalReader::reset(
    const std::shared_ptr<fep3::arya::ISimulationBus::IDataReceiver>& /*receiver*/)
{
}

fep3::Optional<fep3::Timestamp> InternalTopic::InternalReader::getFrontTime() const
{
    return {};
}