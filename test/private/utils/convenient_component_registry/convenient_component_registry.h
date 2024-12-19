/**
 * @copyright
 * @verbatim
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * @endverbatim
 */

#pragma once

#include <fep3/base/component_registry/component_registry.h>

#include <memory>
#include <string>
#include <vector>

namespace fep3::plugin::cpp::catelyn {
class IComponentFactory;
} // namespace fep3::plugin::cpp::catelyn

namespace fep3::test {

struct ComponentRegistry {
    ComponentRegistry(const std::vector<std::string>& native_iids,
                      const std::vector<std::string>& stub_iids);
    ~ComponentRegistry();

    void addNativeComponentToRegistry(const std::string& iid);
    void addStubComponentToRegistry(const std::string& iid);
    void unregisterComponent(const std::string& iid);
    std::shared_ptr<fep3::ComponentRegistry> getRegistry();

private:
    std::unique_ptr<fep3::plugin::cpp::catelyn::IComponentFactory> _native_factory;
    std::unique_ptr<fep3::plugin::cpp::catelyn::IComponentFactory> _stub_factory;
    std::shared_ptr<fep3::ComponentRegistry> _comp_registry;
};

} // namespace fep3::test
