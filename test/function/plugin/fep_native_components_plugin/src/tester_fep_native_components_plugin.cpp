/**
 * @file
 * @copyright
 * @verbatim
Copyright @ 2022 VW Group. All rights reserved.

This Source Code Form is subject to the terms of the Mozilla
Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
@endverbatim
 */

#include <fep3/components/scheduler/scheduler_service_intf.h>
#include <fep3/components/service_bus/service_bus_intf.h>
#include <fep3/plugin/cpp/cpp_plugin_component_factory_intf.h>

#include <fep_components_plugin_test.h>

/**
 * General note on below tests:
 * All function names and types appearing in the plugin interface, are hard coded here rather than
 * taken from the FEP Participant Library exports, because we test the interfaces that are
 * potentially already released. This way we ensure, that if a potentially released interface is
 * changed, the test fails. For example, the (forbidden) change of the name of an existing exported
 * symbol (e. g. "fep3_plugin_cpp_arya_getFactory") will be detected by this test (while it wouldn't
 * be detected if the test would take the symbol name from the FEP Participant Library exports).
 * Such test logic also applies to the IIDs, so the IID's string is hard coded in the tests rather
 * than taken from Interface::FEP3_COMP_IID.
 */

using test::helper::FEPComponentsPluginFixture;

INSTANTIATE_TEST_SUITE_P(FEPNativeComponentsPlugin,
                         FEPComponentsPluginFixture,
                         ::testing::Values(FEP_COMPONENTS_PLUGIN_FILE_PATH));

/**
 * Tests loading components from the fep_components_plugin via fep3_plugin_cpp_arya_getFactory
 */
TEST_P(FEPComponentsPluginFixture, test_arya_createComponent)
{
    auto get_factory_function = get<fep3::plugin::cpp::arya::ICPPPluginComponentFactory*()>(
        "fep3_plugin_cpp_arya_getFactory");
    ASSERT_NE(nullptr, get_factory_function);
    std::unique_ptr<fep3::plugin::cpp::arya::ICPPPluginComponentFactory> component_factory(
        get_factory_function());
    ASSERT_TRUE(component_factory);

    auto arya_clock_service = component_factory->createComponent("clock_service.arya.fep3.iid");
    EXPECT_TRUE(arya_clock_service);

    auto arya_clock_sync_service =
        component_factory->createComponent("clock_sync_service.arya.fep3.iid");
    EXPECT_TRUE(arya_clock_sync_service);

    auto arya_configuration_service =
        component_factory->createComponent("configuration_service.arya.fep3.iid");
    EXPECT_TRUE(arya_configuration_service);

    auto arya_data_registry = component_factory->createComponent("data_registry.arya.fep3.iid");
    EXPECT_TRUE(arya_data_registry);

    auto catelyn_health_service =
        component_factory->createComponent("health_service.catelyn.fep3.iid");
    EXPECT_TRUE(catelyn_health_service);

    auto arya_job_registry = component_factory->createComponent("job_registry.catelyn.fep3.iid");
    EXPECT_TRUE(arya_job_registry);

    auto arya_logging_service = component_factory->createComponent("logging_service.arya.fep3.iid");
    EXPECT_TRUE(arya_logging_service);

    auto arya_scheduler_service =
        component_factory->createComponent("scheduler_service.arya.fep3.iid");
    EXPECT_TRUE(arya_scheduler_service);
    auto catelyn_scheduler_service =
        component_factory->createComponent("scheduler_service.catelyn.fep3.iid");
    EXPECT_TRUE(catelyn_scheduler_service);
    const auto& arya_scheduler_service_interface = static_cast<fep3::arya::ISchedulerService*>(
        arya_scheduler_service->getInterface("scheduler_service.arya.fep3.iid"));
    const auto& catelyn_scheduler_service_interface =
        static_cast<fep3::catelyn::ISchedulerService*>(
            catelyn_scheduler_service->getInterface("scheduler_service.catelyn.fep3.iid"));
    // SchedulerService is a FEP Super Component and fep3::catelyn::ISchedulerService derives from
    // fep3::arya::ISchedulerService, so casting must succeed and the interface pointers must be
    // identical.
    EXPECT_EQ(static_cast<fep3::arya::ISchedulerService*>(catelyn_scheduler_service_interface),
              arya_scheduler_service_interface);

    auto arya_service_bus = component_factory->createComponent("service_bus.arya.fep3.iid");
    EXPECT_TRUE(arya_service_bus);
    auto catelyn_service_bus = component_factory->createComponent("service_bus.catelyn.fep3.iid");
    EXPECT_TRUE(catelyn_service_bus);
    const auto& arya_service_bus_interface = static_cast<fep3::arya::IServiceBus*>(
        arya_service_bus->getInterface("service_bus.arya.fep3.iid"));
    const auto& catelyn_service_bus_interface = static_cast<fep3::catelyn::IServiceBus*>(
        catelyn_service_bus->getInterface("service_bus.catelyn.fep3.iid"));
    // ServiceBus is a FEP Super Component and fep3::catelyn::IServiceBus derives from
    // fep3::arya::IServiceBus, so casting must succeed and the interface pointers must be
    // identical.
    EXPECT_EQ(static_cast<fep3::arya::IServiceBus*>(catelyn_service_bus_interface),
              arya_service_bus_interface);

    auto arya_simulation_bus = component_factory->createComponent("simulation_bus.arya.fep3.iid");
    EXPECT_TRUE(arya_simulation_bus);

    auto arya_participant_info =
        component_factory->createComponent("participant_info.arya.fep3.iid");
    EXPECT_TRUE(arya_participant_info);
}

/**
 * Tests loading components from the fep_components_plugin via fep3_plugin_cpp_catelyn_getFactory
 */
TEST_P(FEPComponentsPluginFixture, test_catelyn_createComponent)
{
    auto get_factory_function =
        get<fep3::plugin::cpp::catelyn::IComponentFactory*()>("fep3_plugin_cpp_catelyn_getFactory");
    ASSERT_NE(nullptr, get_factory_function);
    std::unique_ptr<fep3::plugin::cpp::catelyn::IComponentFactory> component_factory(
        get_factory_function());
    ASSERT_TRUE(component_factory);

    auto arya_clock_service = component_factory->createComponent("clock_service.arya.fep3.iid");
    EXPECT_TRUE(arya_clock_service);

    auto arya_clock_sync_service =
        component_factory->createComponent("clock_sync_service.arya.fep3.iid");
    EXPECT_TRUE(arya_clock_sync_service);

    auto arya_configuration_service =
        component_factory->createComponent("configuration_service.arya.fep3.iid");
    EXPECT_TRUE(arya_configuration_service);

    auto arya_data_registry = component_factory->createComponent("data_registry.arya.fep3.iid");
    EXPECT_TRUE(arya_data_registry);

    auto catelyn_health_service =
        component_factory->createComponent("health_service.catelyn.fep3.iid");
    EXPECT_TRUE(catelyn_health_service);

    auto arya_job_registry = component_factory->createComponent("job_registry.catelyn.fep3.iid");
    EXPECT_TRUE(arya_job_registry);

    auto arya_logging_service = component_factory->createComponent("logging_service.arya.fep3.iid");
    EXPECT_TRUE(arya_logging_service);

    auto arya_scheduler_service =
        component_factory->createComponent("scheduler_service.arya.fep3.iid");
    EXPECT_TRUE(arya_scheduler_service);
    auto catelyn_scheduler_service =
        component_factory->createComponent("scheduler_service.catelyn.fep3.iid");
    EXPECT_TRUE(catelyn_scheduler_service);
    // SchedulerService is a FEP Super Component, so the pointers must be identical.
    EXPECT_EQ(arya_scheduler_service.get(), catelyn_scheduler_service.get());

    auto arya_service_bus = component_factory->createComponent("service_bus.arya.fep3.iid");
    EXPECT_TRUE(arya_service_bus);
    auto catelyn_service_bus = component_factory->createComponent("service_bus.catelyn.fep3.iid");
    EXPECT_TRUE(catelyn_service_bus);
    // ServiceBus is a FEP Super Component, so the pointers must be identical.
    EXPECT_EQ(arya_service_bus.get(), catelyn_service_bus.get());

    auto arya_simulation_bus = component_factory->createComponent("simulation_bus.arya.fep3.iid");
    EXPECT_TRUE(arya_simulation_bus);

    auto arya_participant_info =
        component_factory->createComponent("participant_info.arya.fep3.iid");
    EXPECT_TRUE(arya_participant_info);
}
