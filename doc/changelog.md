<!--
  Copyright 2023 CARIAD SE.
  
This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
-->

# FEP SDK Participant Library Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0) and this project adheres to [Semantic Versioning](https://semver.org/lang/en).

<h3><a href="#fep_sdk_participant">FEP SDK Participant</a></h3>

<!-- DOC GENERATOR HEADER -->
* [3.2.1](#FEP_SDK_3_2_1) \| [Changes](#FEP_SDK_3_2_1_changes) \| [Fixes](#FEP_SDK_3_2_1_fixes) \| [Known issues](#FEP_SDK_3_2_1_known_issues) \| Release date: 2023-11-06
* [3.2.0](#FEP_SDK_3_2_0) \| [Changes](#FEP_SDK_3_2_0_changes) \| [Fixes](#FEP_SDK_3_2_0_fixes) \| [Known issues](#FEP_SDK_3_2_0_known_issues) \| Release date: 2023-07-23
* [3.1.0](#FEP_SDK_3_1_0) \| [Changes](#FEP_SDK_3_1_0_changes) \| [Fixes](#FEP_SDK_3_1_0_fixes) \| [Known issues](#FEP_SDK_3_1_0_known_issues) \| Release date: 2022/04/13
* [3.0.1-beta](#FEP_SDK_3_0_1) \| [Changes](#FEP_SDK_3_0_1_changes) \| [Fixes](#FEP_SDK_3_0_1_fixes) \| [Known issues](#FEP_SDK_3_0_1_known_issues) \| Release date: 2022/02/02
* [3.0.0](#FEP_SDK_3_0_0) \| [Changes](#FEP_SDK_3_0_0_changes) \| [Fixes](#FEP_SDK_3_0_0_fixes) \| [Known issues](#FEP_SDK_3_0_0_known_issues) \| Release date: 2021/03/25


<!-- DOC GENERATOR BODY -->
<a name="FEP_SDK_3_2_1"></a>
 <h3><a href="No url">FEP SDK Participant 3.2.1</a> - Release date: 2023-11-06</h3>

<a name="FEP_SDK_3_2_1_changes"></a>
#### Changes

_**Done**_

- [FEPSDK-3613][] - <a name="FEPSDK-3613_internal_link"></a> Do not export symbols from third party static libraries [\[view\]][125257f785a153a9d00440f1ec592df270279bfd] 
    * Default symbol visibilty set to hidden, some boost symbols could not be hidden
    
    
- [FEPSDK-3713][] - <a name="FEPSDK-3713_internal_link"></a> Improve SystemAccess logs [\[view\]][3c34640c0c6c858ea532e0ddf38727b779833bf5] 
    Logging for service bus improved.
- [FEPSDK-3742][] - <a name="FEPSDK-3742_internal_link"></a> Make scheduler more verbose [\[view\]][5e971bd1aff44ca57e8b0a8d97f376a2b3880da8] 
    * added debug logs in the scheduler service component
    
    
- [FEPSDK-3767][] - <a name="FEPSDK-3767_internal_link"></a> Rename CMake target fep3_system_py 
    Rename target fep3_system_py to fep3_system_py3.10.5
    
<a name="FEP_SDK_3_2_1_fixes"></a>
#### Fixes

_**Done**_

- [FEPSDK-3412][] - <a name="FEPSDK-3412_internal_link"></a> Timing properties reset while state transition deinitialize [\[view\]][a2ff4ce265598b430de53ed9925c2dbb61e19a4e] 
    * Adapted native clock service to not change property values during registration of clocks
    
    
- [FEPSDK-3666][] - <a name="FEPSDK-3666_internal_link"></a> DefaultJobConfigVisitor does not consider queue_size for creation of DataReaders [\[view\]][ad14d9e88d025d6df2d37a1f9be988f12c5bff81] [\[view\]][66c9b657a7a242d84c5366c88a05b45fa921b81a] 
    Adapted DefaultJobConfigVisitor to consider queue_size
    
    
- [FEPSDK-3667][] - <a name="FEPSDK-3667_internal_link"></a> DataTriggeredReceiver invalid warning log results in crash [\[view\]][4ff2b017084d612456582d60881ac9f44f8b8fe0] 
    Fix warning log
- [FEPSDK-3711][] - <a name="FEPSDK-3711_internal_link"></a> Deadlock during ClockService::Stop [\[view\]][47521d46203eec9e373c3db4db9c028fecbedbb3] 
    * simulation clock does not deadlock on stop
    
    
- [FEPSDK-3712][] - <a name="FEPSDK-3712_internal_link"></a> SystemAccessBase::getRequester fails when participant service is running [\[view\]][ac6351e4010e454b7f40e6d8520ee328a22aa90f] 
    * Add multiple tries in getRequester
    
    
- [FEPSDK-3715][] - <a name="FEPSDK-3715_internal_link"></a> Service Bus should process hearbeat messages in the sequence they are received [\[view\]][e0332f744efec1954109df4603bb3f5beddd5474] 
    * Heartbeat message processing fixed
    
    
- [FEPSDK-3716][] - <a name="FEPSDK-3716_internal_link"></a> HttpSystemAccess discovery thread is blocked by synchronous event sink call [\[view\]][8711c7f2dd40af2d42cf8f8fed704f05fed7b782] [\[view\]][93faa43e5f4540da151bbf5179a65e66551219ee] 
    * callables registered with fep3::IServiceBus::IServiceUpdateEventSink are called in a seperate thread and do not block the discovery thread
    
    
- [FEPSDK-3730][] - <a name="FEPSDK-3730_internal_link"></a> New lines in message of Participant incident CSV Log file are not escaped [\[view\]][a18221326dbc5fcd009bdf457320fd7b5d29b1c2] 
    * Line break is escaped. Unit test added.
    
    
- [FEPSDK-3734][] - <a name="FEPSDK-3734_internal_link"></a> Implementation of JobRegistry::getJobs does not retrieve DataTriggeredJobs as documentation suggests [\[view\]][15dea1bdbfe7f405abee3954cf64b790b9bbc7ae] 
    * Difference between fep3::catelyn::IJobRegistry::getJobsCatelyn and fep3::arya::IJobRegistry::getJobs mentioned in documentation
    
    
- [FEPSDK-3763][] - <a name="FEPSDK-3763_internal_link"></a> Potential errors when linking boost targets if boost found via CMake module FindBoost [\[view\]][b27bc354534b273bdcbebd7f8bf21e04408e9d97] 
    * Do not require component 'headers' explicitly in find_package(Boost ...).
    * Correctly apply CMake INTERFACE target 'Boost::disable_autolinking' to affected targets
    * Disables specific compiler warning for included Boost header to not break compilation.

_**Duplicate**_

- [FEPSDK-3756][] - <a name="FEPSDK-3756_internal_link"></a> DataTriggeredReceiver invalid warning log results in crash 

<a name="FEP_SDK_3_2_1_known_issues"></a>

<a name="FEP_SDK_3_2_1"></a>
<!--- Issue links -->
[FEPSDK-3667]
[FEPSDK-3756]
[FEPSDK-3711]
[FEPSDK-3666]
[FEPSDK-3613]
[FEPSDK-3716]
[FEPSDK-3734]
[FEPSDK-3713]
[FEPSDK-3742]
[FEPSDK-3730]
[FEPSDK-3763]
[FEPSDK-3767]
[FEPSDK-3715]
[FEPSDK-3712]
[FEPSDK-3412]


<a name="FEP_SDK_3_2_0"></a>
 <h3><a href="No url">FEP SDK Participant 3.2.0</a> - Release date: 2023-07-23</h3>

<a name="FEP_SDK_3_2_0_changes"></a>
#### Changes

_**Done**_

- [FEPSDK-2367][] - <a name="FEPSDK-2367_internal_link"></a> DataRegistry converts StreamType *fileref* for DDL StreamMetaTypes 
- [FEPSDK-2920][] - <a name="FEPSDK-2920_internal_link"></a> Improve robustness of IElement interface towards component usage 
- [FEPSDK-3001][] - <a name="FEPSDK-3001_internal_link"></a> Provide ability to set additional DDL files for mapping [\[view\]][55531a3f28fec81f651cdf680971a0300f627c81] [\[view\]][28510b619660500b0638aae79bb9b7ff17a87e9a] [\[view\]][e18574d874aae1a6051a7ccc295aa74aa9924c2f] [\[view\]][c64fa6635c7a8e33a6a9ee78875801274cbe0202] [\[view\]][c809e088d835d8a607a1daff0bc187ad92de6e52] [\[view\]][4f0c62278ac4870de76a77c0a7f011b1b2985946] 
    * Participant
         * Added mapping_ddl_file_paths property to provide ddl descriptions via property which will be considered for the native data regitrsy's signal mapping feature
         * Removed mapping_configuration property
         * Made native data registry use Easylogging
         * Fixed some doc/CMake typos
         * Extended error logs in dds simulation bus
         * Test: {{NativeDataRegistryWithMocksBase::testSignalMappingMergeRedundantStruct}} has to be adapted to expect the corresponding error and log once ODAUTIL-573 is implemented.
     * SDK
         * Added signal mapping prose doc section
         * Fixed some typos
    
    
- [FEPSDK-3043][] - <a name="FEPSDK-3043_internal_link"></a> Provide command history 
- [FEPSDK-3058][] - <a name="FEPSDK-3058_internal_link"></a> Document all default commandline argument values in built-in help 
- [FEPSDK-3062][] - <a name="FEPSDK-3062_internal_link"></a> Make FEP 3 participant discovery timeout configurable 
- [FEPSDK-3081][] - <a name="FEPSDK-3081_internal_link"></a> Mark operator>> of DataReader deprecated [\[view\]][479d905f9d95f1ac2cc8bb558a913464fc7cff66] 
    * Marked corresponding DataReader operators deprecated
    * Adapted tests and documentation to use not deprecated alternatives wherever possible
    * Created follow up ticket to removed deprecated operators with 3.2.0
    
    
- [FEPSDK-3100][] - <a name="FEPSDK-3100_internal_link"></a> Implementation line based logging sink [\[view\]][4c4c605897a60f29bd9c58acc83ed668b3b9bc6f] 
- [FEPSDK-3107][] - <a name="FEPSDK-3107_internal_link"></a> Add popSample's'Before(timestamp) to DataReaderBacklog [\[view\]][9dc4a8e462dfbcd8239085ff6366e0fceb713af3] 
    * implemented a new member function in fep3::core::arya::DataReaderBacklog
     data_read_ptr<const fep3::arya::IDataSample> popSamplesBefore(const fep3::arya::Timestamp timestamp)
     * added testcases for all Scenarios in [2021-02-11 Scenarios regarding the reception of signals|https://www.cip.audi.de/wiki/display/FEPSDK/2021-02-11+Scenarios+regarding+the+reception+of+signals?src=jira]
     * extended documentation of DataReader in participant_overview_backup.rst
    
    
- [FEPSDK-3149][] - <a name="FEPSDK-3149_internal_link"></a> Implementation of an alternative discovery mechanism based on unicasting connection 
- [FEPSDK-3173][] - <a name="FEPSDK-3173_internal_link"></a> Support registration of a single instance as multiple FEP Component interfaces from within FEP Component Plugins [\[view\]][52017825005a9692a65c87cb5a5b165ac627d21b] [\[view\]][198afda2aee144aec5639d9fadc083e530e8f98e] [\[view\]][12c5eecd5ac9a332dc4555b2662d2a8797dcb034] 
- [FEPSDK-3174][] - <a name="FEPSDK-3174_internal_link"></a> Implement signal waiting for DDS SimBus [\[view\]][d8b9d76667aa678592da0b9c31ece8644d6027da] 
    * FEP3 participant library code changes
        * rationalize getReader default values
        * delete superfluous code from ReaderWriterTestClass
        * introduce new property variables for datawriter timeout for signals
        * default values of properties to interface header file
        * notification mechanism for connecting writers
        * wildcard handling in signal list
        * test cases to check the feature
        * fix typo
        * get rid of unnecessary arya namespace references
        * delete sleeps from test cases
        * documentation
    
    
- [FEPSDK-3175][] - <a name="FEPSDK-3175_internal_link"></a> [Feature] Convenient data triggered usage [\[view\]][4e03461c8f9c9abc6c193e6825d7d7427ba7711b] [\[view\]][e67636875bb0b1bbd9ea5634058de49a921ad384] [\[view\]][055783fb118890d68ff019ef00d85ff27f46a27b] [\[view\]][e32588b26be2d08495cadad46d71d7d17549ca04] 
- [FEPSDK-3179][] - <a name="FEPSDK-3179_internal_link"></a> Create Health Service ADR 
- [FEPSDK-3218][] - <a name="FEPSDK-3218_internal_link"></a> Remove default loading built-in Component Implementation mechanism [\[view\]][4de50b53f295244dfaab0f15d5f85936c4ddd840] 
    * components moved to a seperate plugin (code remains in same place)
     * Built in load mechanism deactivated
     * INFO message exists but not printed since the logging service is not active yet
     * unit test added
     * ABI Dumper and compl checked modified (now we have to check 2 dlls, participant and fep components plugin dll
     * Announced in dev talk
    
    
- [FEPSDK-3220][] - <a name="FEPSDK-3220_internal_link"></a> Remove all unneeded error codes from fep3_errors.h [\[view\]][1ef427943d28cebe5348ed83dd241aac71611ae3] 
    * Removed all error codes witch were not needed to compile all 4 repos.
     * Error codes present in VU, and the last code left untouched.
    
    
- [FEPSDK-3233][] - <a name="FEPSDK-3233_internal_link"></a> Add uint32 and uint64 standard property datatypes [\[view\]][e7299bbf7c22b5b1e1e1851fdfe424a9b2e36588] [\[view\]][0b31b242d7a88cb32decfff5622e74fc4875e0b8] 
    * Used template metaprogramming for the allowed property types and property conversions
    
    
- [FEPSDK-3252][] - <a name="FEPSDK-3252_internal_link"></a> Implement Healthiness struct [\[view\]][a14d9908e34266de787497dc3700c8392a313d62] [\[view\]][971f26853c4eb9da731cda1fec7a334a1db401b5] 
    * implemented catelyn ISchedulerService and IScheduler
     * example output
    {code:bash}
            fep> callRPC demo_system demo_cpp_receiver health_service health_service.catelyn.fep3.iid getHealth '{}'
            {
                    &quot;action&quot; : &quot;callRPC&quot;,
                    &quot;status&quot; : 0,
                    &quot;value&quot; :
                    {
                            &quot;id&quot; : 1,
                            &quot;jsonrpc&quot; : &quot;2.0&quot;,
                            &quot;result&quot; :
                            {
                                    &quot;jobs_healthiness&quot; :
                                    [
                                            {
                                                    &quot;cycle_time&quot; : 1000000000,
                                                    &quot;job_name&quot; : &quot;receive_job&quot;,
                                                    &quot;last_execute_data_in_error&quot; :
                                                    {
                                                            &quot;error_count&quot; : 0,
                                                            &quot;last_error&quot; :
                                                            {
                                                                    &quot;description&quot; : &quot;&quot;,
                                                                    &quot;error_code&quot; : 0,
                                                                    &quot;file&quot; : &quot;&quot;,
                                                                    &quot;function&quot; : &quot;&quot;,
                                                                    &quot;line&quot; : -1
                                                            },
                                                            &quot;simulation_timestamp&quot; : 0
                                                    },
                                                    &quot;last_execute_data_out_error&quot; :
                                                    {
                                                            &quot;error_count&quot; : 0,
                                                            &quot;last_error&quot; :
                                                            {
                                                                    &quot;description&quot; : &quot;&quot;,
                                                                    &quot;error_code&quot; : 0,
                                                                    &quot;file&quot; : &quot;&quot;,
                                                                    &quot;function&quot; : &quot;&quot;,
                                                                    &quot;line&quot; : -1
                                                            },
                                                            &quot;simulation_timestamp&quot; : 0
                                                    },
                                                    &quot;last_execute_error&quot; :
                                                    {
                                                            &quot;error_count&quot; : 0,
                                                            &quot;last_error&quot; :
                                                            {
                                                                    &quot;description&quot; : &quot;&quot;,
                                                                    &quot;error_code&quot; : 0,
                                                                    &quot;file&quot; : &quot;&quot;,
                                                                    &quot;function&quot; : &quot;&quot;,
                                                                    &quot;line&quot; : -1
                                                            },
                                                            &quot;simulation_timestamp&quot; : 0
                                                    },
                                                    &quot;simulation_timestamp&quot; : 17001385400
                                            }
                                    ]
                            }
                    }
            }
    {code}
    
    
- [FEPSDK-3259][] - <a name="FEPSDK-3259_internal_link"></a> DDS ServiceBus discovery environment [\[view\]][1156cf7712a143fb44e22a360fec117221804e52] 
    * DDS topic used for service discovery, ip is resolved by the hostname at the moment
         * Topic contains host url - > &quot;[http://VWAGWOLXXXXXXX:9090|http://vwagwol38318606:9090/]&quot;
         * participant name &quot;demo_core_receiver@demo_system&quot;
         * message type (bye, discover, heartbeat)
     * Implementation split to base, htts and dds  service bus libs
     * Environment variablle and lsddp made independent libs
     * Change in the implementation is done with a compile time switch for each plugin compilation
     * Documentation updated
     * Some optimization (deletion of unecessary copies) in fep3_base_utilities
     * Switch of the working dir during plugin loading in fep3_system
    
    
- [FEPSDK-3265][] - <a name="FEPSDK-3265_internal_link"></a> [PoC] Realization of Multiple sender transferring to single signal with RTI DDS 
    * In the corresponding Devstack-Ticket ([https://devstack.vwgroup.com/jira/browse/GSILDEV-1143]) multiple sender and receiver of a standard RTI DDS example, which uses the same signal, could exchange frames -> multiple sender for one signal is possible from DDS side
     * Furthermore multiple sender for one signal were tested with a changed FEP SDK example, that sends a counter -> multiple sender for one signal is possible with FEP using DDS as SimulationBus
     * In both cases no sample drops have been seen
    
    
- [FEPSDK-3274][] - <a name="FEPSDK-3274_internal_link"></a> Rename fep_components_plugin.dll 
- [FEPSDK-3275][] - <a name="FEPSDK-3275_internal_link"></a> Remove CMake deployment of fep_component_plugin library [\[view\]][e6bd85d29c5b961be8e4d1d43583a5979d4a80df] [\[view\]][a8f22dcb3b107fa7fb4b9d96663eed18358bcb65] 
    * Added internal_ prefix to macros and added to the comments that the macros will be removed in the future 
    
    
- [FEPSDK-3276][] - <a name="FEPSDK-3276_internal_link"></a> Command line arguments for executables are not aligned with FEP Agent [\[view\]][c90254f7a477c05e16025a6d5791d983f0ea3984] 
    * --system_name and --element_name aliases added to command line arguments
    * positional arguments removed
    * test adjusted to removed positional arguments
    * documentation alignment for participant executable arguments
    
    
- [FEPSDK-3291][] - <a name="FEPSDK-3291_internal_link"></a> Adapt RTI DDS Simulation Bus to use AsyncWaitset instead of Waitset for data reception [\[view\]][3a7e21d326dde72ce7ef001808dd3303dd14f682] 
    * Add AsyncWaitSet to RTI DDS Simulation Bus
     * Use Property to enable the Async WaitSet
     * Adapt all related simulation bus tests with templated test using TYPED_TEST, so that the tests run both on WaiSet and AsyncWaitSet Simulation Bus.
     * Documentation and examples added to fep sdk repo. 
    
    
- [FEPSDK-3292][] - <a name="FEPSDK-3292_internal_link"></a> flipping the order of registering DataReaders and DataWriters within the DataRegistry::tense [\[view\]][3fb7d09057700083e35ed8e66ca496bbdc400659] 
    * test if getWriter calls precede getReader call(s) (testGetReaderWriterOrder)
    * flipping the order of registerAtSimulation bus calls in DataRegistry::tense so that the output signals are registered before input signals
    * non-integrable code to &quot;parking&quot; branches
        * test interconnected simulation buses (TestCircularDependency) -> https://www.cip.audi.de/bitbucket/projects/FEPSDK/repos/fep3_participant/compare/commits?sourceBranch=refs%2Fheads%2Ffeature%2FFEPSDK-3292-circular_dependency_test&targetBranch=refs%2Fheads%2Fdevelop
        * DataRegistry-Simulation bus integration test sketch -> https://www.cip.audi.de/bitbucket/projects/FEPSDK/repos/fep3_participant/compare/commits?sourceBranch=refs%2Fheads%2Ffeature%2FFEPSDK-3292-integration-test-sketch&targetBranch=refs%2Fheads%2Fdevelop
    
    
- [FEPSDK-3328][] - <a name="FEPSDK-3328_internal_link"></a> Provide existing gmocks in FEP repositories as part of the FEP Component API [\[view\]][f52e6ac005649c3c30ae252e4830f54cbb1b90ff] 
    * Moved all mocks implementing FEP Component interfaces (and dependent interfaces) from {*}src{*}/fep3/components/.../mock/... to {*}include{*}/fep3/components/.../mock/...
     * The mock classes have not been in a version namespace yet, so I moved them to the appropriate version namespace and also added the latest version, e. g. by declaring
    {code:java}
    using arya::SimulationBus;{code}
     * Moving the mock classes to the version namespace also requires to derive from an explicit version, so I changed the mock classes to derive from the appropriate interface version (e. g. fep3::{*}arya{*}::IClock)
     * I surrounded the content of the mock files by &quot;///@cond nodoc&quot; and  &quot;///@endcond nodoc&quot; because documenting those doesn't make any sense, as gmock mock classes are self-explaining by design.
     * Some files containing mock classes also contained FEP3-SDK-internal mock implementations (that are most likely not relevant to the FEP3 Component Developer), e. g. mock_clock_service.h:EventSinkTimeEventValues; in order to not make those public, I left them in the appropriate file with postfix &quot;addons&quot;, e. g {*}src{*}/fep3/components/clock/mock/mock_clock_service_addons.h
     * Some of the mock classes are derived from test::helper::Dieable, which I regard to be FEP3-SDK-internal. Therefore these classes went to the appropriate file with postfix &quot;addons&quot;, e. g src/fep3/components/data_registry/mock/mock_data_registry_addons.h
     * Unrelated improvement in separate commit: &quot;[FEPSDK-3328|https://www.cip.audi.de/bitbucket/plugins/servlet/jira-integration/issues/FEPSDK-3328] [FEPSDK-3304|https://www.cip.audi.de/bitbucket/plugins/servlet/jira-integration/issues/FEPSDK-3304] Aligned structure of exported mock classes to the structure of the FEP Component interfaces&quot;, e. g.: ISimulationBus::IDataReader -> mock::SimultionBus::DataReader ( -> made DataReader a subclass of mock::SimulationBus)
     * Unrelated improvement in separate commit &quot;[FEPSDK-3328|https://www.cip.audi.de/bitbucket/plugins/servlet/jira-integration/issues/FEPSDK-3328] [FEPSDK-3304|https://www.cip.audi.de/bitbucket/plugins/servlet/jira-integration/issues/FEPSDK-3304] Switched to new gmock macro &quot;MOCK_METHOD&quot; in public mocks for FEP Component interfaces&quot;
     * Unrelated improvement in separate commit &quot;FEPSDK-3328 FEPSDK-3304 Renamed mock classes for FEP Component interfaces in favor of consistency and removed obsolete duplicates&quot;:
         * fep3::mock::MockComponents -> fep3::mock::Components
         * fep3::mock::DataRegistryComponent -> fep3::mock::DataRegistry
         * fep3::mock::JobRegistryComponent -> fep3::mock::JobRegistry; removed old duplicates fep3::mock::JobRegistry and fep3::mock::JobRegistryComponentBase
         * fep3::mock::MockLoggingService -> fep3::mock::LoggingService; removed old duplicate fep3::mock::LoggingService
         * fep3::mock::MockLogger -> fep3::mock::Logger
         * fep3::mock::ServiceBusComponent -> fep3::mock::ServiceBus
     * Unrelated improvement in separate commit &quot;Removed obsolete include of <boost/bind.hpp> from test/utils/helper/dds_test_service_discovery_helpers.h; this resolves the compiler warning 'The practice of declaring the Bind placeholders (_1, _2, ...) in the global namespace is deprecated.'&quot;
     * Unrelated improvement in separate commit &quot;FEPSDK-3328 FEPSDK-3304 Fixed CMake generator expression syntax&quot;
     * (In accordance to the solution as agreed upon with [~WF36Z4L]  and [~SNSDRCU] , see ticket's comments below): Moved mocks for FEP Component interfaces to fep3_participant/{*}test/{*}include/fep3/components and added CMake target &quot;fep3_components_test&quot;
     * Unrelated improvement in separate commit &quot;FEPSDK-3328 FEPSDK-3304 Moved test-private mocks to appropriate location in order to get rid of weired relative paths in test code&quot;: Some files containing mock classes also contained FEP3-SDK-test-specific mock implementations, e. g. mock_transferable_clock_service_with_access_to_clocks.h (which is specific to the C plugin tests for ClockService interface); in order to not make those public, I moved them to the appropriate test location, e. g {*}test{*}/private/component_intfs/clock/c/src; this improvement also made some hacky relative include paths obsolete, e. g.
    {code:java}
    #include &quot;../../../../../../../src/fep3/components/clock/mock/mock_transferable_clock_service_with_access_to_clocks.h&quot;
    {code}
     * Updated CM plan (actually - as instructed by [~WF36Z4L] - made a copy of the existing CM plan version 3.0 and added the new directory test/include in chapter 6.3): [https://www.cip.audi.de/wiki/display/FEPSDK/CM+plan+version+3.1]
     * Created follow-up ticket for doc and minimal example: FEPSDK-3347
    
    
- [FEPSDK-3342][] - <a name="FEPSDK-3342_internal_link"></a> Add possibility to retrieve loaded components IIDs via RPC [\[view\]][2af596b6d04455b0fc8cb48e10b25837969d8b23] 
    {code:bash}
    	fep> callRPC demo_system demo_cpp_receiver component_registry component_registry.catelyn.fep3.iid getComponentIIDs '{}'
    	{
    			&quot;action&quot; : &quot;callRPC&quot;,
    			&quot;status&quot; : 0,
    			&quot;value&quot; :
    			{
    					&quot;id&quot; : 1,
    					&quot;jsonrpc&quot; : &quot;2.0&quot;,
    					&quot;result&quot; :
    					{
    							&quot;component_iids&quot; : &quot;clock_service.arya.fep3.iid,clock_sync_service.arya.fep3.iid,configuration_service.arya.fep3.iid,data_registry.arya.fep3.iid,health_service.catelyn.fep3.iid,job_registry.arya.fep3.iid,logging_service.arya.fep3.iid,participant_info.arya.fep3.iid,scheduler_service.catelyn.fep3.iid,service_bus.catelyn.fep3.iid,simulation_bus.arya.fep3.iid&quot;
    					}
    			}
    	}
    	fep> callRPC demo_system demo_cpp_receiver component_registry component_registry.catelyn.fep3.iid getFilePath '{&quot;service_iid&quot; : &quot;service_bus.catelyn.fep3.iid&quot;}'
    	{
    			&quot;action&quot; : &quot;callRPC&quot;,
    			&quot;status&quot; : 0,
    			&quot;value&quot; :
    			{
    					&quot;id&quot; : 1,
    					&quot;jsonrpc&quot; : &quot;2.0&quot;,
    					&quot;result&quot; :
    					{
    							&quot;file_path&quot; : &quot;C:/Dev\\fep3_sdk\\build\\package\\examples\\bin\\rti\\fep3_dds_service_bus_plugin&quot;
    					}
    			}
    	}
    	fep> callRPC demo_system demo_cpp_receiver component_registry component_registry.catelyn.fep3.iid getFilePath '{&quot;service_iid&quot; : &quot;data_registry.arya.fep3.iid&quot;}'
    	{
    			&quot;action&quot; : &quot;callRPC&quot;,
    			&quot;status&quot; : 0,
    			&quot;value&quot; :
    			{
    					&quot;id&quot; : 1,
    					&quot;jsonrpc&quot; : &quot;2.0&quot;,
    					&quot;result&quot; :
    					{
    							&quot;file_path&quot; : &quot;C:/Dev\\fep3_sdk\\build\\package\\examples\\bin\\fep_components_plugin&quot;
    					}
    			}
    	}
    {code}
    
    
- [FEPSDK-3358][] - <a name="FEPSDK-3358_internal_link"></a> Update C Component Interface with new catelyn::ISchedulerService, IScheduler, ISchedulerRegistry 
- [FEPSDK-3464][] - <a name="FEPSDK-3464_internal_link"></a> Stream Type DDL should correctly select Qos Setting according to size [\[view\]][c3b565eb5cb3000ef1f96fe139b3799d23a177a9] [\[view\]][687dbcc3f339535ed962f2e13bf0b7aaa43dac2c] [\[view\]][60719329bb54f3837b0144736795bdf6c6f7eb0f] [\[view\]][9790fddebd0d18089a697c11f17de2fa4ac57553] [\[view\]][0fcaa7a13b7f54640d57a5a6713fc0189283a2da] 
    * Adapted RTI DDS Simulation Bus to use &quot;*_big&quot; qos profiles for DDL signals >= 64kb
    
    
- [FEPSDK-3482][] - <a name="FEPSDK-3482_internal_link"></a> Change RTI DDS signal connection default behavior to "wait for writer" 
- [FEPSDK-3504][] - <a name="FEPSDK-3504_internal_link"></a> Use persistent integration builds in FEP Products [\[view\]][09a5354518ded9c4a97d45e0d6cc3531db2f97bd] 
    * Use set_version from CoRTEXPythonRequiresHelper
     * Use always the lateset integration build
     * Script for creation of Release notes
     * Script for update and PR creation for updating release branch 
    
    
- [FEPSDK-3508][] - <a name="FEPSDK-3508_internal_link"></a> Automatically generate release notes [\[view\]][f3e880cc01c2fab16652f72c4277de2b0bc2032f] 
    * Using CortexPythonRequiresHelper to create the changelog for the current version
    
    
- [FEPSDK-3512][] - <a name="FEPSDK-3512_internal_link"></a> Measure and Improve Throughput performance of Data Job 
    * DataJob support added.
     * Complex combination of signals can be configured, please see readme and help
     * Same system, different topics possible.
     * Send and Receive simutanously in one participant is possible. 
     * More details in output if more than one signal is configured. Please see readme. See attached pictures also.
     * The throughput can be reached at a level of 10 Gbps with HighTrhoughput QoS profile. 
    
    
- [FEPSDK-3514][] - <a name="FEPSDK-3514_internal_link"></a> FEP3 32 bit [\[view\]][f161e8aeb3e3a1c72523de715aa1956008904bbd] [\[view\]][7ae7aaece6eabd0e56aee0183f9602658ed8fcfd] 
    * Added pipeline for x86 with gcc7
     * The following package variants are available
         * fep_sdk_participant/3.2.0 - Linux_x86_gcc7
         * fep_sdk_system/3.2.0 - Linux_x86_gcc7
         * fep_sdk/3.2.0 - Linux_x86_gcc7
    
    
- [FEPSDK-3520][] - <a name="FEPSDK-3520_internal_link"></a> Smoke Test 32 - 64 Bit Communication 
    * How to install 32bit package
    
    {code:java}
    conan install fep_sdk/3.2.0@fep/FEPSDK_3514 -pr Linux_x86_gcc7 -u -o fep_sdk_system:python_version=
    {code}
    The python_version has to be left empty
     * How to add 32bit support in a 64bit Linux to run the binary.
    
    {code:java}
    sudo dpkg --add-architecture i386
    sudo apt-get update
    sudo apt-get install libc6:i386 libncurses5:i386 libstdc++6:i386
    {code}
     * data_triggered example works. 
     * demo_signal_mapping example not working, probably because of alignment.
    !image-2022-11-18-13-45-30-907.png!
     * demo_easy_core example not working
    !image-2022-11-18-13-44-31-316.png!
     *  demo_easy_cpp example not working
    !image-2022-11-18-13-47-58-839.png!
    
    
- [FEPSDK-3521][] - <a name="FEPSDK-3521_internal_link"></a> ADTF Player Participant time update frequency shall be configurable 
    * Solved with ACORE-11799 &quot;Custom FEP clock cStreamClockAsFepClock shall use time barriers to participate in scheduling&quot;
    
    
- [FEPSDK-3522][] - <a name="FEPSDK-3522_internal_link"></a> Improve Service Bus logging for debugging [\[view\]][d854801220607810e560e0408a991786bc559ef4] 
- [FEPSDK-3550][] - <a name="FEPSDK-3550_internal_link"></a> Const Job configuration cannot be added to job registry [\[view\]][78c20b424e48fdc9b7dee20ecf3d21683a88bc1e] 
    * Const JobConfiguration as parameter
    
    
- [FEPSDK-3551][] - <a name="FEPSDK-3551_internal_link"></a> Data Triggered job should reconfigurable via Properties and vice versa [\[view\]][bdce9d2bce73c5b5e5c0a2775d85c077d0744044] [\[view\]][a352402a94b3e429aeb6fb9c4e6301982f797a0d] 
    * Adapted job registry to create all job configuration properties regardless of the job trigger type
    
    
- [FEPSDK-3555][] - <a name="FEPSDK-3555_internal_link"></a> Clean up fep sdk participant includes [\[view\]][e82d3e5cf89f47f5209fa560cc6401a8022130f4] [\[view\]][620bc7bcedecf7f1da8d14e5e677363ebae0045d] [\[view\]][18769e320092120deab084016a519f74981e13cc] [\[view\]][97fd55a05c81df05c4983290e4bd29378f371e99] [\[view\]][74c290453c052dabd7aa3f30ddff16f9cd64cd40] 
    * Cleans up includes
    * Removed unnecessary standard header includes like iostream, vector, etc.
    * Removes unused cpp files
    
    
- [FEPSDK-3557][] - <a name="FEPSDK-3557_internal_link"></a> Provide Convenient Element and Job class [\[view\]][99ea0f21f1d5cf0d8d8e749edf85ac253e3e712f] 
    * DefaultJobElement added
     * CustomJobElement added
     * DefaultJob added
     * Add default virtual function for convenience API
     * Enable Rollback of DataReaders added
     * Enable Rollback of DataWriters added
     * Remove unnecessary comments
     * JobConfiguration params from DefaultJob
     * IDataIOContainer added
     * DataIOContainer
     * CustomElementFactory unit test
     * Documentation for public API added
     * CustomElementFactory provided
     * Unit test adapted
     * mock classes for tests
     * Provide CustomElementFactory for customer argument
     * Reconfiguration of jobs and data signals is possible
     * Move classes to core
     * Unit tests and system tests added
     * Documentation, examples, code snippets and best practices added.
    
    
- [FEPSDK-3577][] - <a name="FEPSDK-3577_internal_link"></a> DDS Discovery messages should send the IP instead of host name in case default Server Url ist not used  [\[view\]][e75082368c87d7084d7f9fd10ee8c1ce89d28c24] [\[view\]][273256281778e154c89b9b35cc33e061236bd7b0] 
- [FEPSDK-3581][] - <a name="FEPSDK-3581_internal_link"></a> Registering DataReader for a signal that is input to mapping should be possible [\[view\]][a7b42248587f13b6ba0731c5ebe8dfdf404ea822] [\[view\]][21bc657a8caab81e9015d772eaa69c81910dc7f4] 
    * Registering DataReader for a signal thas is input to mapping is now possible independent of the order of registering.
     * Documentation is adapted. 
    
    
- [FEPSDK-3585][] - <a name="FEPSDK-3585_internal_link"></a> Refactor Local Clock Based Scheduler [\[view\]][d8d5e3bf9bc97c7e667eaaa423d4e0a8ec6f5235] 
    * Scheduler jobs executed in thread pool with maximum 4 threads
     * Bugfix when the scheduler was deinitialized and re-started, then the job was not triggered at 0ms
    
    
- [FEPSDK-3596][] - <a name="FEPSDK-3596_internal_link"></a> Example of signal with ddl structure containing dynamic array [\[view\]][651ea9d0e122015f66335b3d1d39e8c2f8691a45] 
    * Example using DDL dynamic array
     * Converted error to warning when using signal including a DDL dynamic array
    
    
- [FEPSDK-3601][] - <a name="FEPSDK-3601_internal_link"></a> Data Triggered Jobs should run on scheduler thread pool [\[view\]][da86b7e0d06a7af2957926df1bb3ee26019fdba4] 
    * Option 1 realized.
     * Thread pool in scheduler implemented
     * DataTrigger task will be executed in thread pool
     * Thread pool can be started and stopped separately.
    
    
- [FEPSDK-3604][] - <a name="FEPSDK-3604_internal_link"></a> Add logging sink file_csv [\[view\]][b87f03371e094d2f4685bb7b64e7d5f630511799] [\[view\]][0820ff654a2299d445a64ccba033044bab2461f1] 
    * Added csv based logging format which is used by logging sink &quot;file&quot;
    * Replaced &quot;log_type&quot; by &quot;simulation_time&quot; in json logging format
    
     
- [FEPSDK-3607][] - <a name="FEPSDK-3607_internal_link"></a> Participant State Machine RPC Server returns error descriptions on failed transitions [\[view\]][0a8671da7066707f4acaef5dae6a0ff7738f3781] 
    * participant lib:
         * rpc participant state machine transitions return fep3::result
     * system lib:
         * health_proxy.hpp: fixed json to fep3::result conversion
         * creates an RPC State Machine Client depending on the iid of the state_machine RPC Service
         * FEPSDK-3625 
    
    
- [FEPSDK-3609][] - <a name="FEPSDK-3609_internal_link"></a> State Machine does not rollback correctly on component registry or element transition error [\[view\]][bd83c1f7b84e6b12589e6740b8a1e3a41f1f84e6] [\[view\]][ba26724e8190152a3bef880015693d10f32476e0] 
    * Element is stoped when ComponentRegistry::start returns a non zero Result 
     * Element is deinitialized when component_registry::initialized returns a non zero Result 
     * Element and component registry are deinitialized when component_registry::tense returns a non zero Result
    
    
- [FEPSDK-3651][] - <a name="FEPSDK-3651_internal_link"></a> Copy service_bus symbol files while calling deploy [\[view\]][85616f2d2169bb507d6824f37ac7f01142588d8f] [\[view\]][2c391fee38e6a6f6b467642ccd34b1f700d0f394] 
    The CMake macro internal_fep3_participant_deploy additionally copies symbol files of MSVC dynamic libraries to a TARGET_FOLDER.
    
    

_**Duplicate**_

- [FEPSDK-2849][] - <a name="FEPSDK-2849_internal_link"></a> [PoC] Add an event distribution mechanism for changes of participant health states 
- [FEPSDK-2934][] - <a name="FEPSDK-2934_internal_link"></a> StreamType big for DDL 
- [FEPSDK-3465][] - <a name="FEPSDK-3465_internal_link"></a> User defined QoS Profiles can be used in participants 
- [FEPSDK-3472][] - <a name="FEPSDK-3472_internal_link"></a> Remove transmission of StreamType for static DataWriter 

_**Duplikat**_

- [FEPSDK-3442][] - <a name="FEPSDK-3442_internal_link"></a> Data triggered job works with multiple signals 

_**Incomplete**_

- [FEPSDK-3568][] - <a name="FEPSDK-3568_internal_link"></a> Support DevHub2 for build pipelines 

_**Won't Do**_

- [FEPSDK-2039][] - <a name="FEPSDK-2039_internal_link"></a> Extend the current clock synchronisation with maximal time step check 
- [FEPSDK-2264][] - <a name="FEPSDK-2264_internal_link"></a> Extend HTTP System Access and its Service Discovery with better unique Service Names 
- [FEPSDK-2501][] - <a name="FEPSDK-2501_internal_link"></a> Unexpected behaviour regarding fep control tool help 
- [FEPSDK-2580][] - <a name="FEPSDK-2580_internal_link"></a> Improve LoggingService filter configuration 
- [FEPSDK-2672][] - <a name="FEPSDK-2672_internal_link"></a> Record if logging with logging service fails 
- [FEPSDK-3030][] - <a name="FEPSDK-3030_internal_link"></a> IRPCRequester::sendRequest interface should be noexcept 
- [FEPSDK-3031][] - <a name="FEPSDK-3031_internal_link"></a> Change templatization level of fep3::cpp::arya::DataJobElement 
- [FEPSDK-3033][] - <a name="FEPSDK-3033_internal_link"></a> Improve log message time formatting 
- [FEPSDK-3057][] - <a name="FEPSDK-3057_internal_link"></a> Call ::fep3::cpp::DataJob::initConfiguration from within fep3::cpp::addToComponents 
- [FEPSDK-3369][] - <a name="FEPSDK-3369_internal_link"></a> Automatically do safe type-casts i.e. for int32 on int64 properties 
- [FEPSDK-3425][] - <a name="FEPSDK-3425_internal_link"></a> Provide functionality to disable RTI DDS Domain Tag within RTI DDS SimulationBus plugin 
- [FEPSDK-3454][] - <a name="FEPSDK-3454_internal_link"></a> Investigate timing with slower participants 
<a name="FEP_SDK_3_2_0_fixes"></a>
#### Fixes

_**Cannot Reproduce**_

- [FEPSDK-3131][] - <a name="FEPSDK-3131_internal_link"></a> Operating a FEP Participant on CARIAD Windows client may fail 
- [FEPSDK-3352][] - <a name="FEPSDK-3352_internal_link"></a> getPropertyValue optional default_value does not work correctly 

_**Done**_

- [FEPSDK-2557][] - <a name="FEPSDK-2557_internal_link"></a> DataWriter and DataReader API correction [\[view\]][0aaa80444a7996875b3c1709eacf7575ca947abb] [\[view\]][ea834797fcd6e85da1a236535c0b977cc8577a51] [\[view\]][e21e2af843b7270fdd5ec918614bd97a2c9c5a33] 
- [FEPSDK-2642][] - <a name="FEPSDK-2642_internal_link"></a> Result::getErrorLabel() mostly returns "(unknown)" 
- [FEPSDK-2786][] - <a name="FEPSDK-2786_internal_link"></a> FEP 3 Timing  provides negative timestamp after re-initialization [\[view\]][f029522d5857886a57b2803076ea34c8f6103ce7] 
- [FEPSDK-2879][] - <a name="FEPSDK-2879_internal_link"></a> Loss of first samples after start of simulation 
- [FEPSDK-2918][] - <a name="FEPSDK-2918_internal_link"></a> Equivalent behaviour of addToDataRegistry and removefromDataRegistry [\[view\]][ca28f2f3e6fb320ca231e1aad0c31278aeacdbb6] [\[view\]][3e5fddbe354297c9dbb4d424d602579a2f004c8e] 
- [FEPSDK-2983][] - <a name="FEPSDK-2983_internal_link"></a> Deinitializing FEP Participant will lead to DDS exception 
- [FEPSDK-3027][] - <a name="FEPSDK-3027_internal_link"></a> Useless error message "Error: (unknown)" when trying to set a non existing property via fep_control 
    * FEP 3 System
        * Extended getProperties error log and exception by description
        * Unrelated changes
            * Added descriptions to results being logged
            * Added doc/build to .gitignore
    * FEP 3 Base Utilities
        * Added test for get/set invalid property path
    
    
- [FEPSDK-3064][] - <a name="FEPSDK-3064_internal_link"></a> IElement nonmutable calls are not const-correct [\[view\]][72924f2211b2d6e469caac2fd60812e163f40f41] 
    * IElement's and ElementBase's getVersion and getTypeName are const now
    
    
- [FEPSDK-3066][] - <a name="FEPSDK-3066_internal_link"></a> Improve the robustness of base classes by complying with the "Rule of Five" 
    * Explicitly deleted Copy/Move CTOR/assignment operator unless already explicitly implemented and added DTOR for base classes.
    * Make Copy/Move CTOR/assignment operator protected for polymorphic base classes as recommended in https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Rc-copy-virtual if they are explicitly defaulted.
    * Adapted base classes to &quot;rule of five&quot; and slicing problem
    
    
- [FEPSDK-3069][] - <a name="FEPSDK-3069_internal_link"></a> Incorrect wording in FEP Control API for shutdown 
    * FEP3 Base Utilities' docs already removed from FEP SDK
     * &quot;shutdowned&quot; occurences in FEP3 System Library fixed
     * &quot;shutdowned&quot; occurences in FEP3 Base Utilities fixed
    
    
- [FEPSDK-3092][] - <a name="FEPSDK-3092_internal_link"></a> ServiceBus' logger initialization missing [\[view\]][c728420c850d69edc490333c70e68873cef45f34] 
- [FEPSDK-3097][] - <a name="FEPSDK-3097_internal_link"></a> Race condition in participant http TCP port allocation [\[view\]][68110cc32befacf736e54976bf35171243ae9ca3] [\[view\]][96b8c067ea1980f6908b9e87732ecfbe3ba9dfc3] [\[view\]][30b47b19c376413e5f00b5b209dda9ec760b837c] [\[view\]][65337d8c33ea22ebdee408ba29ef890f75598460] 
- [FEPSDK-3117][] - <a name="FEPSDK-3117_internal_link"></a> Deregistration of signals fails in combination with mapping functionality [\[view\]][b9e3a02b5cbe2d3078645ed30f0220fc0c43d8d9] 
    * Adapted native data registry's 'removeDataIn' to unregister mapped DataIns.
     * Adapted test 'testSignalMappingIn':
         * to check deregistration of the mapped target signal which is registered at the beginning of the test.
         * to not register the mapped source signal manually as this shall be done by the mapping component automatically.
    
    
- [FEPSDK-3162][] - <a name="FEPSDK-3162_internal_link"></a> Config cmake files have typo [\[view\]][68edf52da86e05b6dd492205457d95cdc958c8fd] 
    * Fixed typos in cmake config files.
- [FEPSDK-3212][] - <a name="FEPSDK-3212_internal_link"></a> ContinuousClock::getTime() leads to infinite recursive loop [\[view\]][bb13cc9f0b2f15d05e6782a707397f2cd1a7a587] 
    * Added a flag so that during a time reset a second reset is not executed.
    
    h1. Review
    
    [~TEJK6U3]
- [FEPSDK-3217][] - <a name="FEPSDK-3217_internal_link"></a> purgeAndPopSampleBefore does not work as intended (always returns nullptr) [\[view\]][c54a70e345aa8f5544a34cee2a0cfe38b8848566] 
    * Fixed function to behave as described within the scenario (Confluence) and documentation
    * Extended tests
    
    
- [FEPSDK-3228][] - <a name="FEPSDK-3228_internal_link"></a> Wrong relative path resolution while loading Component Plugins [\[view\]][ef7ba694283f611fe0d81d4fc2dfe9e70ec64ebd] 
    * problem was that the fep_components file path used for relative path resolution of the plugin contained a dot 
    {code:java}
    C:\\Dev\\fep3_sdk\\build\\examples\\src\\easy_core_example_element\\Debug\\.\\fep3_participant.fep_components{code}
    
           as a result , instead of going two paths up, the code was including the &quot;.&quot; as a directory, so in reality only a directory one path up was calculated.
     * Boost filesystem is used, first the canonical path of the fep_components file is calculated and then the realative path of the plugin is calculated to an absolute.
    
    
- [FEPSDK-3232][] - <a name="FEPSDK-3232_internal_link"></a> JSON file log only valid in shutdown 
- [FEPSDK-3243][] - <a name="FEPSDK-3243_internal_link"></a> Logging is delayed on VW WindowsClients [\[view\]][c12be57c6375b39d0fe15c3361a87fc66d77e1bf] 
    * Solution described in [comment| https://www.cip.audi.de/jira/browse/FEPSDK-3243?focusedCommentId=955212&page=com.atlassian.jira.plugin.system.issuetabpanels%3Acomment-tabpanel#comment-955212]
    * Solution presented to [~W5WI6VN]
    *
    
    
- [FEPSDK-3261][] - <a name="FEPSDK-3261_internal_link"></a> Enable the discovery on specific network interfaces [\[view\]][34e0ad8afa6589359d4991efbbf0ed6058b40105] [\[view\]][c5e647856f4a5813ca73e3f02208915a9da63a87] 
    * Fix sendResponse, it binds to the wrong host.
     * We can specify the interface now using an environment variable.
     * Tested on clients machine
    
     
    
- [FEPSDK-3263][] - <a name="FEPSDK-3263_internal_link"></a> Component loading does not work (in some cases ) if using pthread [\[view\]][c2d7409103f633849894790522b45eccde2da6d1] 
    * Added macro for linking pthread
    * added documentation in the fep_sdk
    
    
- [FEPSDK-3277][] - <a name="FEPSDK-3277_internal_link"></a> Race condition in response of getMasterTime (continuous system time) [\[view\]][3251d259e1faf0b65436ff69078a4b7ef6b03013] 
    * To prevent the time shift to occur, the code in the class ContinuousClock that triggers a reset during a running simulation was removed. Only a reset at the beginning of the simulation is necessary because in continuous timing mode the clocks are monotonically rising.
    
    
- [FEPSDK-3294][] - <a name="FEPSDK-3294_internal_link"></a> DataWriter::flushNow method should check if pointer exist 
- [FEPSDK-3317][] - <a name="FEPSDK-3317_internal_link"></a> Logging sink properties are setable but cannot be queried [\[view\]][43670c1acd430f4ef995fd466f8610fa9729bfad] 
    * Property is set when log file could be opened and is appendable
    
    
- [FEPSDK-3336][] - <a name="FEPSDK-3336_internal_link"></a> Timing master triggers client and own jobs in parallel [\[view\]][eeb1cac04f4aca11e58dfbc9932ec869d64db6ba] 
- [FEPSDK-3340][] - <a name="FEPSDK-3340_internal_link"></a> Comma separator for getStreamType RPC call creates problems when a value has a comma inside [\[view\]][dba9ed39059a14c69ad24e53f12329d3580da898] [\[view\]][f18b372cba5b2af45dcc0eec68f51338ae667d77] 
    * Adapted RPC interface of data registry component to use JSON Arrays to transmit properties via getStreamType
     * Adapted System library RPC client to data registry intf changes
     * Added example to data registry prose doc
     * Fixed some typos/links in the sdk component prose doc section
    
    
- [FEPSDK-3346][] - <a name="FEPSDK-3346_internal_link"></a> Error message and usage string for system name are inconsistent 
- [FEPSDK-3360][] - <a name="FEPSDK-3360_internal_link"></a> Configuration Service is not accessible from within FEP Component C plugins 
- [FEPSDK-3427][] - <a name="FEPSDK-3427_internal_link"></a> Deadlock during RPC Server destruction [\[view\]][fb8372bebfd16095d1edf1e88bbcd12aea9c539d] [\[view\]][d6b75a0efc53ed3063c2b7d29e6b1e39b0762489] 
- [FEPSDK-3433][] - <a name="FEPSDK-3433_internal_link"></a> Usage of commas in description files breaks DDL StreamType 
    * Implemented with FEPSDK-3340
- [FEPSDK-3451][] - <a name="FEPSDK-3451_internal_link"></a> Registering big sized structs as FEP signal failing 
- [FEPSDK-3463][] - <a name="FEPSDK-3463_internal_link"></a> Problems with switching components from a 3.1 built participant to 3.2 using arya 
- [FEPSDK-3468][] - <a name="FEPSDK-3468_internal_link"></a> Libraries depend on generated JSON stubs which is not reflected by CMake targets [\[view\]][c1327db8811cf8eab429394dd1e6c2426efbf0ba] 
    * New cmake function available for generating rpc stubs that make generation a target dependendency,.
    
    
- [FEPSDK-3469][] - <a name="FEPSDK-3469_internal_link"></a> Plugin binary names with dot cannot be loaded [\[view\]][8e8a6c5f9a06b8857e9c748fd31457a774d835c3] 
- [FEPSDK-3478][] - <a name="FEPSDK-3478_internal_link"></a> Mapped signals are not unregistered from mapping engine [\[view\]][d8e7122043d4330cef4281c1857466da5dfcaf01] 
    * Adapted data registry to remove mapped signals from the mapping engine on removal.
    * Adapted internal mapping return values to be consistent
    * Added new/missing return values to data registry documentation
    * Added test regarding reinitialization of participant using mapping
    
    
- [FEPSDK-3483][] - <a name="FEPSDK-3483_internal_link"></a> error message not meaningful while adding/merging a ddl description [\[view\]][4976d8cf589775d85f2473e0ce0595e489f581b7] 
    * Adapted ddl manager to return info regarding problems which occurred during parsing of a ddl string
    
    
- [FEPSDK-3484][] - <a name="FEPSDK-3484_internal_link"></a> Job of the timing master is triggered at wrong time directly after entering the "running" state [\[view\]][3b7ec166f2b7d11e696ceab192bc607bea61f6ee] 
    * FEP System
        * Removed timing master participant from determinism tester
    * FEP Participant
        * Adapted native discrete clock to distribute timestamp 0 and trigger jobs at timestamp 0
        * Adapted scheduler to not trigger jobs on start and time reset anymore
    
    
    
- [FEPSDK-3494][] - <a name="FEPSDK-3494_internal_link"></a> Unsuccessful initialization when using feature "waitforsignal" 
    * the time unit of property *rti_dds_simulation_bus/datawriter_ready_timeout* is *nanosecond* . *1 nanosecond = 1E-09 second*
     * If a non-existen signal is defined in the property {*}rti_dds_simulation_bus/must_be_ready_signals{*}, this signal will just be ignored, no error, no warning.
     * * in propery *rti_dds_simulation_bus/datawriter_ready_timeout* means to wait all signals which defined in participant.
    
    
- [FEPSDK-3530][] - <a name="FEPSDK-3530_internal_link"></a> DDS Host name resolving fails on Jenkins [\[view\]][ce1cb075a0073c19f5b3a4ba2fe47af5801a3497] 
    * Add {{boost::asio::ip::resolver_query_base::numeric_service}} to
    {code:java}
    tcp::resolver::query query(host, PORT, boost::asio::ip::resolver_query_base::numeric_service); 
    {code}
    
    
- [FEPSDK-3562][] - <a name="FEPSDK-3562_internal_link"></a> Disable copy of fep3::base::arya::Configuration  [\[view\]][ead3e14821fe3b19da8bad4e2c737b6732598dae] 
    * fep3::base::arya::Configuration copy and copy assignment ctor marked as deprecated 
     * ConnextDDSSimulationBus public interface getConfiguration() removed (uses copy ctor of class Configuration)
    
    
- [FEPSDK-3576][] - <a name="FEPSDK-3576_internal_link"></a> Discrete Timing time factor scales waiting incorrectly [\[view\]][83f226723e86e196e4b9d7d35edd2deece9a916e] 
- [FEPSDK-3589][] - <a name="FEPSDK-3589_internal_link"></a> job_registry is not a FEP Super Component [\[view\]][7cb6c0101ef490bbf343eb496301a5b1cac9a594] 
- [FEPSDK-3591][] - <a name="FEPSDK-3591_internal_link"></a> Timing master waits for jobs according to their expected completion time [\[view\]][2cfeb5c0140d1d92fd97a3f9fe90b1aa82698a25] 
    * Time master (sync. task executor) waits for a job when:
         * its period is set to 0 (one shot task)
         * the next time update event timestamp is negative or the current time is equal or greater to the next execution time (invalid next time update event timestamp)
         * its next job execution timestamp is smaller or equal to the next time update event timestamp
     * Time update event (master/slave) interface expanded by an optional new next update event timestamp 
         * new catelyn IClock, IClockRegistry and IClockService interface
         * data io container interface changed (catelyn::IClockService instead of arya::IClockService)
         * generic clock variant handling implemented (arya/catelyn IClock adapter)
         * RPC interface expanded 
     * Bug was occuring when the clock slave [asks the type of master clock | https://devstack.vwgroup.com/bitbucket/projects/FEPSDK/repos/fep3_participant/browse/src/fep3/native_components/clock_sync/master_on_demand_clock_client.cpp?until=ef3627e83589b794aee2fef76c7501cca5ff0ef4&untilPath=src%2Ffep3%2Fnative_components%2Fclock_sync%2Fmaster_on_demand_clock_client.cpp#136] (but timing master clock was not yet initialized)
    
    
- [FEPSDK-3593][] - <a name="FEPSDK-3593_internal_link"></a> Fix copy semantics of fep3::arya::PropertyValueWithObserver class [\[view\]][f228d8a0fd6b55ca2b57b4eaef1a9294b3d259b4] 
    * disabled copy constructor, copy assignment operator, move constructor and move assignment operator
     * If the user want to copy this, please write their own copy functions. 
    {code:java}
    void copyTo(SimToolInterfaceProperties& copy) 
    void copyTo(SimToolInterfaceProperties& copy, IConfigurationService config_service)
    {code}
    
    
- [FEPSDK-3595][] - <a name="FEPSDK-3595_internal_link"></a> OpenSource: CMake build of FEP SDK examples fails  [\[view\]][0b3370880b857b10cfce836396995e4fa39d145b] 
    * Fixed documentation
    
    
- [FEPSDK-3603][] - <a name="FEPSDK-3603_internal_link"></a> Fix error "mutex destroyed while still in use" in DDS ServiceBus  [\[view\]][7f88c482aaf789d420d6d3081e077204e08811a3] 
    * The mutex has been removed
     * Afterwards it was verified that the error message couldn't be reconstructed
    
    
- [FEPSDK-3619][] - <a name="FEPSDK-3619_internal_link"></a> Default Job API did not remove job from job registry while unloading  [\[view\]][326a16f32cc3e63a4ffcb1f4f8a2195d55b1ab96] 
- [FEPSDK-3630][] - <a name="FEPSDK-3630_internal_link"></a> Health Service RPC Service does not respond with valid json object when job registry is empty [\[view\]][2fa9f78d6ad09250440776691e9c60af21ca8a09] 
    * Exception avoided
     * Missing test added
     * Test in fep system also added
    
    
- [FEPSDK-3655][] - <a name="FEPSDK-3655_internal_link"></a> slave_master_on_demand clock does not always call IEventSink reset events [\[view\]][d4a35596721b49a0ef7e62c96421e46d5b178b09] 
    * Reset event is cached in case the slave clock is not started
    * Event is forwarded on clock start
    
    

_**Duplicate**_

- [FEPSDK-3281][] - <a name="FEPSDK-3281_internal_link"></a> Scheduling starts before all FEP Participants are ready to run 

_**Duplikat**_

- [FEPSDK-3411][] - <a name="FEPSDK-3411_internal_link"></a> Invalid data received by deinitialization of participant 

_**Not A Bug**_

- [FEPSDK-3493][] - <a name="FEPSDK-3493_internal_link"></a> Different timing behaviour in FEP SDK 3.2 during state running 
    * Behaviour is reproducible using
        * the provided system.yml and the feature branch versions of the elements
        * the default discrete time sync configuration (100ms cycle time, 1.0 time factor)
        * a rather slow discrete time sync configuration (10ms cycle time, 1.0 time factor)
        * another rather slow discrete time sync configuration (50ms cycle time, 0.5 time factor)
        * a more verbose configuration (debug severity==debug and show_cmd=True for all Participants)
    -> Logs indicate correct registration of timing slaves at timing master and correct distribution of timing updates for all timing slaves
        * a system extended by FEP Element ADTF Scope
    -> Scope shows Back/FrontDistance jump to inf right after first simulation stept and result in driver position x jumping to inf which then proceeds to upcoming participants via the position signal
    --> source might be division by 0 here: https://www.cip.audi.de/bitbucket/projects/FEPINT/repos/fep3_timing_example/browse/driver/src/driver_participant.cpp?at=refs%2Ftags%2Fv3.0.0#56
    --> question is why this never happened pre FEP SDK 3.2.0
    * Timing Example behaviour using FEP SDK 3.1.0
        * Driver Participant skips here until second simulation step (200ms simulation time): https://www.cip.audi.de/bitbucket/projects/FEPINT/repos/fep3_timing_example/browse/driver/src/driver_participant.cpp?at=refs%2Ftags%2Fv3.0.0#141
        * never runs into divison by 0 problem as stated above
    * Timing Example behaviour using FEP SDK 3.2.0
        * Driver Participant does not skip here https://www.cip.audi.de/bitbucket/projects/FEPINT/repos/fep3_timing_example/browse/driver/src/driver_participant.cpp?at=refs%2Fheads%2Ffeature%2FFEPINT-2462-migrate-timing-example-to-fep-sdk-3.2#141
    * Differences between FEP SDK 3.1.0 and FEP SDK 3.2.0
        * In previous versions no optional was used and therefore an error has been returned which resulted in the processing step (e.g. the first step with simulation time 0 which in the end results in the inf acceleration) being skipped: https://www.cip.audi.de/bitbucket/projects/FEPINT/repos/fep3_timing_example/browse/src/utils.h?at=refs%2Ftags%2Fv3.0.0#98
        * the new version does not skip the first steps as the optional does not have a value and therefore does not return an error: https://www.cip.audi.de/bitbucket/projects/FEPINT/repos/fep3_timing_example/browse/src/utils.h?at=refs%2Fheads%2Ffeature%2FFEPINT-2462-migrate-timing-example-to-fep-sdk-3.2#99
    * Proposed fix for Timing Example: https://www.cip.audi.de/bitbucket/projects/FEPINT/repos/fep3_timing_example/commits/9726632460fadc286c935137307038ff3ae0fd00
    
    

_**Won't Do**_

- [FEPSDK-2957][] - <a name="FEPSDK-2957_internal_link"></a> Pressing key UP may execute command 

<a name="FEP_SDK_3_2_0_known_issues"></a>

<a name="FEP_SDK_3_2_0"></a>
<!--- Issue links -->
[FEPSDK-3521]
[FEPSDK-3291]
[FEPSDK-3332]
[FEPSDK-3604]
[FEPSDK-3107]
[FEPSDK-3342]
[FEPSDK-3233]
[FEPSDK-3369]
[FEPSDK-3508]
[FEPSDK-3057]
[FEPSDK-3482]
[FEPSDK-3031]
[FEPSDK-3555]
[FEPSDK-3340]
[FEPSDK-3276]
[FEPSDK-3263]
[FEPSDK-3162]
[FEPSDK-3360]
[FEPSDK-3550]
[FEPSDK-3212]
[FEPSDK-3651]
[FEPSDK-3179]
[FEPSDK-3577]
[FEPSDK-3530]
[FEPSDK-3259]
[FEPSDK-3601]
[FEPSDK-3551]
[FEPSDK-3442]
[FEPSDK-2367]
[FEPSDK-2557]
[FEPSDK-3294]
[FEPSDK-3427]
[FEPSDK-3619]
[FEPSDK-3256]
[FEPSDK-2983]
[FEPSDK-3117]
[FEPSDK-3493]
[FEPSDK-3562]
[FEPSDK-3576]
[FEPSDK-3058]
[FEPSDK-3261]
[FEPSDK-2918]
[FEPSDK-3346]
[FEPSDK-3102]
[FEPSDK-3596]
[FEPSDK-2264]
[FEPSDK-2039]
[FEPSDK-2786]
[FEPSDK-3514]
[FEPSDK-3593]
[FEPSDK-3603]
[FEPSDK-3630]
[FEPSDK-3064]
[FEPSDK-3030]
[FEPSDK-3252]
[FEPSDK-3174]
[FEPSDK-3100]
[FEPSDK-3149]
[FEPSDK-2580]
[FEPSDK-3522]
[FEPSDK-3033]
[FEPSDK-2920]
[FEPSDK-3066]
[FEPSDK-3069]
[FEPSDK-3411]
[FEPSDK-3454]
[FEPSDK-3232]
[FEPSDK-3484]
[FEPSDK-3468]
[FEPSDK-3243]
[FEPSDK-3317]
[FEPSDK-2879]
[FEPSDK-3062]
[FEPSDK-3478]
[FEPSDK-3081]
[FEPSDK-3512]
[FEPSDK-3595]
[FEPSDK-3131]
[FEPSDK-3607]
[FEPSDK-3469]
[FEPSDK-2957]
[FEPSDK-3463]
[FEPSDK-3557]
[FEPSDK-3001]
[FEPSDK-3043]
[FEPSDK-3328]
[FEPSDK-3425]
[FEPSDK-3097]
[FEPSDK-3277]
[FEPSDK-2672]
[FEPSDK-3585]
[FEPSDK-3581]
[FEPSDK-3451]
[FEPSDK-3275]
[FEPSDK-3220]
[FEPSDK-3218]
[FEPSDK-3472]
[FEPSDK-3274]
[FEPSDK-2642]
[FEPSDK-3281]
[FEPSDK-3092]
[FEPSDK-3520]
[FEPSDK-3253]
[FEPSDK-3609]
[FEPSDK-3464]
[FEPSDK-2934]
[FEPSDK-3568]
[FEPSDK-3173]
[FEPSDK-3336]
[FEPSDK-3591]
[FEPSDK-2501]
[FEPSDK-3494]
[FEPSDK-3358]
[FEPSDK-3445]
[FEPSDK-3433]
[FEPSDK-3504]
[FEPSDK-3027]
[FEPSDK-3465]
[FEPSDK-3228]
[FEPSDK-3175]
[FEPSDK-2849]
[FEPSDK-3265]
[FEPSDK-3483]
[FEPSDK-3292]
[FEPSDK-3352]
[FEPSDK-3589]
[FEPSDK-3217]
[FEPSDK-3655]

<a name="FEP_SDK_3_1_0"></a>
## [3.1.0]

<a name="FEP_SDK_3_1_0_changes"></a>
#### Changes

- FEPSDK-3278 update to c++17
- FEPSDK-3239 Provide plugin versions via RPC
- FEPSDK-3311 Update to dev_essential 1.1.1
- FEPSDK-3100 added new sink for json file logging

<a name="FEP_SDK_3_1_0_fixes"></a>
### Bugfixes
- FEPSDK-3317 Logging sink properties are setable but cannot be queried

<a name="FEP_SDK_3_0_1"></a>
## [3.0.1-beta]

<a name="FEP_SDK_3_0_1_changes"></a>
### Changes
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

<a name="FEP_SDK_3_0_1_fixes"></a>
### Bugfixes
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

<a name="FEP_SDK_3_0_0"></a>
## [3.0.0]

<a name="FEP_SDK_3_0_0_changes"></a>
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
