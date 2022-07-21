<!--
  Copyright @ 2021 VW Group. All rights reserved.
  
      This Source Code Form is subject to the terms of the Mozilla
      Public License, v. 2.0. If a copy of the MPL was not distributed
      with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
  
  If it is not possible or desirable to put the notice in a particular file, then
  You may include the notice in a location (such as a LICENSE file in a
  relevant directory) where a recipient would be likely to look for such a notice.
  
  You may add additional accurate notices of copyright ownership.
  
  -->

# FEP SDK Participant Library Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0) and this project adheres to [Semantic Versioning](https://semver.org/lang/en).

## [Unreleased]

## [3.1.0]

### Changes
- FEPSDK-3278 update to c++17
- FEPSDK-3239 Provide plugin versions via RPC
- FEPSDK-3311 Update to dev_essential 1.1.1
- FEPSDK-3100 added new sink for json file logging
- FEPSDK-3255 Performance changes, added async stop to lssdpcpp, jsonconnector cache, optimized http_system_access
- FEPSDK-3253 Refined documentation of Simulation Bus interface in order to describe real time capability
- FEPSDK-3239 implemented component registry rpc service for retrieving component version
- FEPSDK-3230 Update to dev_essential 1.1.0
- FEPSDK-3218 Remove default loading built-in Component Implementation mechanism
- FEPSDK-3190 Fixed clock sync service intf doc
- FEPSDK-3137 Improve fep sdk timing documentation
- FEPSDK-3106 Update to dev_essential 1.0.0
- FEPSDK-3103 Add Windows_x64_vc142_VS2019 build profile
- FEPSDK-3101 Add Linux_armv8_gcc7 build profile
- FEPSDK-3051 Update to pkg_rpc 3.5.1
- FEPSDK-2923 Define allowed character for signal names
- FEPSDK-2910 Feature Signal Renaming - Documentation and Example

### Bugfixes
- FEPSDK-3317 Logging sink properties are setable but cannot be queried
- FEPSDK-3277 Fix race condition for continuous timing
- FEPSDK-3263 Add macro for linking pthread
- FEPSDK-3261 Enable the discovery on specific interfaces
- FEPSDK-3243 use ip 127.0.0.1 instead of hostname in case participant and server are in the same machine
- FEPSDK-3228 Fix relative path resolution when loading plugin, addapted and added registry factory unit tests
- FEPSDK-3212 ContinuousClock::getTime() leads to infinite recursive loop
- FEPSDK-3162 Fixed typos in cmake config files
- FEPSDK-3117 Adapted data registry's 'removeDataIn' to unregister mapped DataIns
- FEPSDK-3108 Adapt native simulation bus component to clear internal storages of readers/writers/transmitter on deinitialization
- FEPSDK-3097 Resolve the race condition in TCP port allocation between different element processes
- FEPSDK-3091 Corrected/improved documentation of DataWriter and DataSampleType
- FEPSDK-3073 Adapted clock service to return errors in case of invalid native discrete clock configuration and extended log messages.
- FEPSDK-3072 DataItemQueue::nextTime returns nothing if the 1st item in queue is StreamType
- FEPSDK-3070 Fix misleading error code in cpp-api
- FEPSDK-3055 cpp::createParticipant documentation is wrong
- FEPSDK-3049 Native Timing Master returns wrong clock type
- FEPSDK-3048 DataReader's extraction operator>> does handle empty queue correctly
- FEPSDK-3032 fep3::core::arya::ElementBase::getComponents() potentially returns uninitialized/dangling pointer
- FEPSDK-3028 Class ::fep3::cpp::arya::DataJobElement should register itself in load transition to make job configurable via properties
- FEPSDK-3008 Run time check against max_runtime_real_time / max_run_realtime is inaccurate on Linux
- FEPSDK-2996 Missleading error log for missconfigured property clock/step_size
- FEPSDK-2995 Access to renaming properties defect
- FEPSDK-2992 getStreamType ignores alias names
- FEPSDK-2988 Doc and implementation differs for scheduler name
- FEPSDK-2949 Memory Corruption at process termination with gcc 5.4.0
- FEPSDK-2936 Dataregistry reports intermediate signals over RPC
- FEPSDK-2918 Equivalent behaviour of addToDataRegistry and removefromDataRegistry
- FEPSDK-2917 Fix doc of DataRegistry
- FEPSDK-2871 Participant uses high cpu load on Linux 
- FEPSDK-2860 Negative Timestamp in JobRunner::runJob()
- FEPSDK-2856 fix thread safe DomainParticipant constructor call and changing of working dirs
- FEPSDK-2671 Refactor and document exception handling of the service bus

## [3.0.0]

### Changes
- FEPSDK-2916 Fix properties to use nanoseconds unit 
- FEPSDK-2865 Allow job to be set programmatically in DataJobElement
- FEPSDK-2857 Set pause mode as unsupported
- FEPSDK-2779 Make parameters of fep3::getPropertyValue const
- FEPSDK-2768 Rename property cycle_time_ms of clock service 
- FEPSDK-2755 Add C wrapper for FEP Component "Configuration Service"
- FEPSDK-2727 Perform thorough performance testing
- FEPSDK-2719 Add C wrapper for FEP Component "Participant Info"
- FEPSDK-2710 Move CommandLineParser class to the core library
- FEPSDK-2703 Add missing StreamType classes
- FEPSDK-2701 Use Boost Circular Buffer for DataItemQueue instead of custom implementation
- FEPSDK-2662 Redesign class DataReaderBacklog : public IDataRegistry::IDataReceiver
- FEPSDK-2585 Prepare FEP3 SDK code to be distributed as OSS
- FEPSDK-2577 Move timing slave registration for continuous clock synchronization to initialization state
- FEPSDK-2565 Adapt native components to communicate via interfaces instead of properties
- FEPSDK-2561 [FEP3] Use the new profiles [gcc5, v141] as base for the delivery packages
- FEPSDK-2525 Create createParticipant() variant that parses the argc / argv arguments
- FEPSDK-2489 System name should change the dds domain id
- FEPSDK-2487 Add C wrapper for component Job Registry
- FEPSDK-2486 Increase verbosity of ClockService and ClockSyncService
- FEPSDK-2480 Clock based scheduler in continuous mode should wait for a time reset event before starting scheduling
- FEPSDK-2479 Perform thorough performance testing
- FEPSDK-2427 Add user information on Component loading
- FEPSDK-2411 Implement RPC for job registry native component
- FEPSDK-2406 Support for QoS for user defined stream types
- FEPSDK-2402 Improve error handling and error information of Participant::exec
- FEPSDK-2352 Logging Service must react on changes for default configuration while initialize()
- FEPSDK-2348 Separate PropertyNode and Properties
- FEPSDK-2344 Components need to know the participant name (and maybe other information)
- FEPSDK-2329 DDS should support multiple domain instances in one local network
- FEPSDK-2327 Late joiner should be provided with streamtype informations
- FEPSDK-2320 Integrate RPC Configuration Service to FEP System API 3
- FEPSDK-2319 Integrate configuration service and IPropertyNode (with e.g. scheduler, clock service)
- FEPSDK-2301 Create initial documentation
- FEPSDK-2262 Add detailed user documentation on FEP Component System
- FEPSDK-2244 Split StreamMetaType meta_type_ddl
- FEPSDK-2191 Dynamic Array Support within DataRegistry/SimBus
- FEPSDK-2148 Create FEP3 native SimulationBus Component Implementation based on RTI DDS Connext 6
- FEPSDK-2131 Add discover mechanism with ssd protocol to the HTTP System Access implementation
- FEPSDK-2124 Implement RPC for clock service and native component
- FEPSDK-2078 Implement usage of _delay_sim_time within clock based scheduler
- FEPSDK-2037 Implement RPC for scheduler service native component
- FEPSDK-1996 Reduce usage of class Timestamp (having validity information) to std::chrono::nanoseconds (or alias) whereever possible
- FEPSDK-1985 Introduce timestamp supporting validity indication
- FEPSDK-1917 Add C++ Plugin Mechanism to Participant Library and a Components File
- FEPSDK-1916 Make the Component "SimulationBus" an exchangeable component
- FEPSDK-1908 Make the Component "SchedulerService" an exchangeable component
- FEPSDK-1340 [POC] Design a Health Service for FEP Participant Library
- FEPSDK-1319 Design and Implementation for C++ Service Bus Interface (IServiceBus)
- FEPSDK-1314 Prepare FEP SDK code to be distributed as OSS
- FEPSDK-1291 Provide the mapping capability
- FEPSDK-1247 Add c plugin mechanism for FEP3 component system
- FEPSDK-1233 Add FEP 3 RPC State Machine Service + Integrate RPC Interface into FEP System Library
- FEPSDK-1210 Design for C++ Simulation Bus Interface
