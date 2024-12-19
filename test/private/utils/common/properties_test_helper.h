/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <fep3/base/properties/propertynode.h>

inline fep3::base::NativePropertyNode createTestProperty(
    const std::string& node_name = std::string("Clock"))
{
    fep3::base::NativePropertyNode properties_clock(node_name);

    auto node_clocks = std::make_shared<fep3::base::NativePropertyNode>("Clocks", int32_t{2});

    auto node_clocks1 =
        std::make_shared<fep3::base::NativePropertyNode>("Clock1", std::string{"my name"});
    node_clocks1->setChild(
        std::make_shared<fep3::base::NativePropertyNode>("CycleTime", int32_t{1}));

    properties_clock.setChild(node_clocks);
    node_clocks->setChild(node_clocks1);

    return properties_clock;
}

inline std::shared_ptr<fep3::IPropertyNode> createTestProperties(
    const std::string& node_name = std::string("Clock"))
{
    auto properties_clock = std::make_shared<fep3::base::NativePropertyNode>(node_name);

    auto node_clocks = std::make_shared<fep3::base::NativePropertyNode>("Clocks", int32_t{2});

    auto node_clocks1 =
        std::make_shared<fep3::base::NativePropertyNode>("Clock1", std::string{"my name"});

    node_clocks1->setChild(
        std::make_shared<fep3::base::NativePropertyNode>("CycleTime", int32_t{1}));

    auto node_clocks2 = std::make_shared<fep3::base::NativePropertyNode>("Clock2");
    node_clocks2->setChild(
        std::make_shared<fep3::base::NativePropertyNode>("CycleTime", int32_t{2}));

    properties_clock->setChild(node_clocks);
    node_clocks->setChild(node_clocks1);
    node_clocks->setChild(node_clocks2);

    return properties_clock;
}

inline std::shared_ptr<fep3::IPropertyNode> createTypeTestProperties()
{
    auto node_types = std::make_shared<fep3::base::NativePropertyNode>("types");
    node_types->setChild(fep3::base::makeNativePropertyNode<int32_t>("int", 1));
    node_types->setChild(fep3::base::makeNativePropertyNode<double>("double", 1.0));
    node_types->setChild(fep3::base::makeNativePropertyNode<bool>("bool", true));
    node_types->setChild(fep3::base::makeNativePropertyNode<std::string>("bool", "some value"));

    return node_types;
}

inline std::shared_ptr<fep3::base::NativePropertyNode> setChildImpl(
    std::shared_ptr<fep3::base::NativePropertyNode> node,
    std::shared_ptr<fep3::base::NativePropertyNode> to_add)
{
    node->setChild(to_add);
    return to_add;
}
