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


#include <fep3/cpp/datajob.h>

namespace
{
/**
 * @brief addDataReaders Adds all given data readers to the given data registry.
 * Returns instantly if an error occurs while adding the data readers.
 * @param data_readers Data readers to add to the data registry.
 * @param data_registry Data registry to add the data readers to.
 * @param rollback Bool whether an error has occurred.
 * @return fep3::Result
 */
fep3::Result addDataReaders(std::list<fep3::core::arya::DataReader>& data_readers,
	fep3::IDataRegistry& data_registry,
	bool& rollback)
{
	fep3::Result result;

	for (auto& reader : data_readers)
	{
		result = reader.addToDataRegistry(data_registry);
		if (fep3::isFailed(result))
		{
			rollback = true;
			return result;
		}
	}

	return {};
}

/**
 * @brief addDataRWriters Adds all given data writers to the given data registry.
 * Returns instantly if an error occurs while adding the data readers.
 * @param data_writers Data writers to add to the data registry.
 * @param data_registry Data registry to add the data writers to.
 * @param rollback Bool whether an error has occurred.
 * @return fep3::Result
 */
fep3::Result addDataWriters(std::list<fep3::core::arya::DataWriter>& data_writers,
	fep3::IDataRegistry& data_registry,
	bool& rollback,
	fep3::IClockService& clock_service)
{
	fep3::Result result;

	for (auto& writer : data_writers)
	{
		result = writer.addToDataRegistry(data_registry);
		if (isFailed(result))
		{
			rollback = true;
			return result;
		}

		result = writer.addClock(clock_service);
		if (isFailed(result))
		{
			rollback = true;
            return result;
		}
	}

	return {};
}

/**
 * @brief removeDataReaders Remove all given data readers from a given data registry.
 * Does not return if an error occurs. Stores all errors which occurred during removal
 * of readers.
 * @param data_readers Data readers to remove.
 * @param data_registry Data registry to remove data readers from.
 * @param results Vector of data reader and error pairs.
 */
void removeDataReaders(std::list<fep3::core::arya::DataReader>& data_readers,
	fep3::IDataRegistry& data_registry,
	std::vector<std::pair<std::string, fep3::Result>>& results)
{
	for (auto& reader : data_readers)
	{
		auto result = reader.removeFromDataRegistry(data_registry);

		if (fep3::isFailed(result))
		{
			results.emplace_back(reader.getName(), result);

		}
	}
}

/**
 * @brief removeDataWriters Remove all given data writers from a given data registry.
 * Does not return if an error occurs. Stores all errors which occurred during removal
 * of writers.
 * @param data_writer Data writers to remove.
 * @param data_registry Data registry to remove data writers from.
 * @param results Vector of data writer and error pairs.
 */
void removeDataWriters(std::list<fep3::core::arya::DataWriter>& data_writers,
	fep3::IDataRegistry& data_registry,
	std::vector<std::pair<std::string, fep3::Result>>& results)
{
	for (auto& writer : data_writers)
	{
		auto result = writer.removeFromDataRegistry(data_registry);
		if (isFailed(result))
		{
			results.emplace_back(writer.getName(), result);
		}

		result = writer.removeClock();
		if (isFailed(result))
		{
			results.emplace_back(writer.getName(), result);
		}
	}
}
}

namespace fep3
{
namespace cpp
{
namespace arya
{

DataJob::DataJob(const std::string& name, fep3::arya::Duration cycle_time) :
    Configuration("job_" + name),
    Job(name, cycle_time,
        [&](Timestamp time_of_ex) { return process(time_of_ex); })
{
}

DataJob::DataJob(const std::string& name, fep3::arya::JobConfiguration job_config) :
    Configuration("job_" + name),
    Job(name, job_config,
        [&](Timestamp time_of_ex) { return process(time_of_ex); })
{
}

DataJob::DataJob(const std::string& name, fep3::arya::Duration cycle_time, DataJob::ExecuteCallback fc) :
    Configuration("job_" + name),
    Job(name, cycle_time, fc)
{
}

DataJob::DataJob(const std::string& name, fep3::arya::JobConfiguration job_config, DataJob::ExecuteCallback fc) :
    Configuration("job_" + name),
    Job(name, job_config, fc)
{
}

DataReader * DataJob::addDataIn(const std::string& name, const IStreamType & type)
{
    _readers.push_back(DataReader(name, type));
    return &_readers.back();
}

DataReader * DataJob::addDataIn(const std::string& name, const IStreamType & type, size_t queue_size)
{
    _readers.push_back(DataReader(name, type, queue_size));
    return &_readers.back();
}

DataWriter * DataJob::addDataOut(const std::string& name, const IStreamType & type)
{
    _writers.push_back(DataWriter(name, type, core::arya::DATA_WRITER_QUEUE_SIZE_DEFAULT));
    return &_writers.back();
}

DataWriter * DataJob::addDynamicDataOut(const std::string& name, const IStreamType & type)
{
    _writers.push_back(DataWriter(name, type, core::arya::DATA_WRITER_QUEUE_SIZE_DYNAMIC));
    return &_writers.back();
}

fep3::Result DataJob::reconfigureDataIn(const std::string& name, size_t queue_size)
{
    for (auto& reader : _readers)
    {
        if (reader.getName() == name)
        {
            reader.setCapacity(queue_size);
            return {};
        }
    }
    return fep3::Result(ERR_NOT_FOUND);
}

DataWriter * DataJob::addDataOut(const std::string& name, const IStreamType & type, size_t queue_size)
{
    if (0 >= queue_size)
    {
        throw std::runtime_error("a queue size <= 0 is not valid");
    }

    _writers.push_back(DataWriter(name, type, queue_size));
    return &_writers.back();
}

fep3::Result DataJob::process(Timestamp /*time_of_execution*/)
{
    return {};
}

fep3::Result DataJob::reset()
{
    return Job::reset();
}

fep3::Result DataJob::executeDataIn(Timestamp time_of_execution)
{
    for (auto& current : _readers)
    {
        current.receiveNow(time_of_execution);
    }
    return {};
}

fep3::Result DataJob::executeDataOut(Timestamp time_of_execution)
{
    fep3::Result res{};
    for (auto& current : _writers)
    {
        //this will empty the data writer queues and write the data usually to the simulation bus
        //usually it will also call the transimission of the queue content
        //this will usually NOT wait for data really transmitted and
        //it will usually NOT wait for the response of the receivers of the data
        res |= current.flushNow(time_of_execution);
    }
    return res;

}

fep3::Result DataJob::addDataToComponents(const IComponents& components)
{
    bool rollback = false;
    fep3::Result res;

    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry == nullptr)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "Datajob needs IDataRegistry, but not found in component");
    }
    auto clock_service = components.getComponent<IClockService>();
    if (clock_service == nullptr)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "Datajob needs IClockService, but not found in component");
    }

    if (!rollback)
    {
        res = initLogger(components, "element.job." + getJobInfo().getName());
        if (isFailed(res))
        {
            rollback = true;
        }
    }

    if (!rollback)
    {
		res = addDataReaders(_readers, *data_registry, rollback);
    }

    if (!rollback)
    {
		res = addDataWriters(_writers, *data_registry, rollback, *clock_service);
    }

    if (rollback)
    {
		FEP3_LOG_ERROR(a_util::strings::format("An error occurred during registration of data writers and data readers at the data registry. '%d' - '%s'. Trying to rollback.",
			res.getErrorCode(),
			res.getDescription()));

        // result is discarded as we want to return the error which occurred during registration
        removeDataFromComponents(components);

        return res;
    }
    else
    {
        return {};
    }
}

fep3::Result DataJob::removeDataFromComponents()
{
    for (auto& reader : _readers)
    {
#include <fep3/base/compiler_warnings/disable_deprecation_warning.h>
        reader.removeFromDataRegistry();
#include <fep3/base/compiler_warnings/enable_deprecation_warning.h>
    }
    for (auto& writer : _writers)
    {
#include <fep3/base/compiler_warnings/disable_deprecation_warning.h>
        writer.removeFromDataRegistry();
#include <fep3/base/compiler_warnings/enable_deprecation_warning.h>
        writer.removeClock();
    }
    deinitLogger();
    return {};
}

fep3::Result DataJob::removeDataFromComponents(const IComponents& components)
{
    auto data_registry = components.getComponent<IDataRegistry>();
    if (data_registry == nullptr)
    {
        RETURN_ERROR_DESCRIPTION(ERR_INVALID_ADDRESS, "Datajob needs IDataRegistry, but not found in component");
    }

	std::vector<std::pair<std::string, fep3::Result>> results;

	removeDataReaders(_readers, *data_registry, results);
	removeDataWriters(_writers, *data_registry, results);

	for (const auto& result_entry : results)
	{
        FEP3_LOG_ERROR(a_util::strings::format("Removing '%s' failed because of: '%d' - '%s'",
                                               result_entry.first.c_str(),
                                               result_entry.second.getErrorCode(),
                                               result_entry.second.getDescription()));
	}

    deinitLogger();

	if (1 == results.size())
	{
		return results.front().second;
	}
	else if (1 < results.size())
	{
		RETURN_ERROR_DESCRIPTION(ERR_FAILED,
			a_util::strings::format("Multiple errors occurred during removal of data writers and data readers from data job '%s'. Have a look at the logs for further information.",
				getJobInfo().getName().c_str()).c_str());
	}

    return {};
}

}
}
}
