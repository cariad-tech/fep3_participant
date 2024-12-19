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

#include <fep3/base/component_plugin_analyzer/component_description_base.h>
#include <fep3/components/base/component_intf.h>
#include <fep3/components/base/components_intf.h>
#include <fep3/fep3_errors.h>

#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

namespace fep3::base {
class ComponentRegistrySpy;
class PropertyTester;
class SignalTester;

/**
 * @brief The class tracks the calls to
 * FindComponent done by each component
 * Checks the properties that are added after each component state transition call.
 * Checks the readers and writers that are added after each component state transition call.
 */
class ComponentPluginAnalyzer {
public:
    /**
     * @brief Constructor.
     *
     * @param components_to_analyze The components to analyze. The set of components should
     * be complete, meaning that with this set the state machine can be cycled.
     */
    ComponentPluginAnalyzer(const std::vector<ComponentToAnalyze>& components_to_analyze);

    ~ComponentPluginAnalyzer();
    /**
     * @brief Returns the data after the analysis. Call this at the end
     * after all the @ref analyzeStateTransition and @ref analyzeComponentFunction
     * are done. After this call no more calls to @ref analyzeStateTransition and @ref
     * analyzeComponentFunction are possible.
     * @return The metamodel information of the components.
     */
    std::vector<ComponentMetaModelInfo> getData();

    /**
     * @brief .Calls an invokable for each component. If an invokable returns an error
     * the loop is stopped and the error is returned. No rollback is performed.
     *
     * @param callable The state transition function that will be called for each component.
     * @param next_state The state that the components will have after the call.
     * @return True if no error, otherwise the error returned by the @p callable
     */
    fep3::Result analyzeStateTransition(
        std::function<fep3::Result(std::shared_ptr<fep3::IComponents>, fep3::IComponent&)> callable,
        const std::string& next_state);

    /**
     * @brief .Invokes a callable on a component. Invoke this if you want to
     * set for example properties during state transitions, or you want to
     * include an additional component call to the component analysis.
     *
     * @param callable The callable that should be invoked on the component.
     * @tparam T The type of the component that the @p callable expects
     * @tparam R The return type of the callable.
     * @return The value returned from the @p callable if the type is convertible,
     * otherwise no error will be returned..
     */
    template <typename T, typename R>
    fep3::Result analyzeComponentFunction(std::function<R(T&)> callable)
    {
        const std::string iid = (fep3::getComponentIID<T>());
        auto comp = static_cast<T*>(findComponent(iid)->getInterface(iid));
        if (!comp)
            return CREATE_ERROR_DESCRIPTION(
                fep3::ERR_POINTER, "Invalid component iid", fep3::getComponentIID<T>().c_str());
        if constexpr (std::is_convertible_v<R, fep3::Result>) {
            fep3::Result ret;
            triggerComponentAndAnalyze([&]() { ret = callable(*comp); }, iid);
            return ret;
        }
        else {
            triggerComponentAndAnalyze([&]() { callable(*comp); }, iid);
            return {};
        }
    }

    /**
     * @brief Prints the metamodel information in a format compatible to the
     * components metamodel yml. However it has some limitations, for example
     * the property values (including these of the stream types) cannot contain :.
     *
     * @param meta_info The metamodel information of the components.
     */
    void print(const std::vector<ComponentMetaModelInfo>& meta_info);

private:
    ///@cond nodoc
    void addComponentsToSpy(const std::vector<ComponentToAnalyze>& components_to_analyze);
    std::unordered_set<ComponentPath> getAllComponentFilePaths(
        const std::vector<ComponentToAnalyze>& components_to_analyze);

    fep3::Result triggerComponentRegistryAndAnalyze(
        std::function<fep3::Result(fep3::IComponent&)> callable, const std::string& next_state);

    void triggerComponentAndAnalyze(std::function<void()> callable, const std::string& iid);

    fep3::arya::IComponent* findComponent(const std::string& fep_iid);

    std::shared_ptr<ComponentRegistrySpy> _spy;
    std::vector<std::string> _component_files;
    std::unique_ptr<PropertyTester> _prop_tester;
    std::unique_ptr<SignalTester> _sig_tester;
    ///@endcond nodoc
};
} // namespace fep3::base
