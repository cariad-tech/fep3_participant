/**
 * Copyright @ 2023 VW Group. All rights reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/components/base/mock_components.h>
#include <fep3/core.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
using Components = NiceMock<fep3::mock::Components>;

struct CustomElementFactoryTest : Test {
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    int int_expected = 1;
    int int_ref_expected = 1;
    std::string string_expected = "";
    std::string string_ref_expected = "";
    std::shared_ptr<int> shared_ptr_expected = std::make_shared<int>(1);
    std::unique_ptr<int> unique_ptr_expected = std::make_unique<int>(1);
    std::shared_ptr<Components> components = std::make_shared<Components>();
};

class MyJob : public fep3::core::DefaultJob {
public:
    MyJob() : fep3::core::DefaultJob("my_job")
    {
    }

    void createDataIOs(const fep3::arya::IComponents&,
                       fep3::core::IDataIOContainer&,
                       const fep3::catelyn::JobConfiguration&) override
    {
    }

    fep3::Result execute(fep3::Timestamp) override
    {
        return {};
    }
};

class MyJobElement : public fep3::core::CustomJobElement {
public:
    MyJobElement(int value_int,
                 int& value_int_ref,
                 int value_int_literal,
                 std::string value_string,
                 std::string& value_string_ref,
                 const std::string& value_string_literal,
                 std::shared_ptr<int> shared_ptr_int,
                 std::unique_ptr<int> unique_ptr_int)
        : _value_int(value_int),
          _value_int_ref(value_int_ref),
          _value_int_literal(value_int_literal),
          _value_string(value_string),
          _value_string_ref(value_string_ref),
          _value_string_literal(value_string_literal),
          _shared_ptr_int(shared_ptr_int),
          _unique_ptr_int(std::move(unique_ptr_int)),
          CustomJobElement("job_element")

    {
        (void)_value_int;
        (void)_value_int_ref;
        (void)_value_int_literal;
        (void)_value_string_ref;
    }

    std::string getTypename() const override
    {
        return "job_element";
    }
    std::string getVersion() const override
    {
        return "1.0.0";
    }

    std::tuple<fep3::Result, JobPtr, JobConfigPtr> createJob() override
    {
        using namespace std::chrono_literals;

        return {fep3::Result{},
                std::make_shared<MyJob>(),
                std::make_unique<fep3::ClockTriggeredJobConfiguration>(100ms)};
    }

    fep3::Result destroyJob() override
    {
        return {};
    }

private:
    int _value_int;
    int& _value_int_ref;
    int _value_int_literal;
    std::string _value_string;
    std::string& _value_string_ref;
    std::string _value_string_literal;
    std::shared_ptr<int> _shared_ptr_int;
    std::unique_ptr<int> _unique_ptr_int;
};

TEST_F(CustomElementFactoryTest, CTOR_successful)
{
    // It should compile and pass the correct arguments
    fep3::core::CustomElementFactory<MyJobElement>(int_expected,
                                                   std::ref(int_ref_expected),
                                                   1,
                                                   string_expected,
                                                   std::ref(string_ref_expected),
                                                   "string literal",
                                                   shared_ptr_expected,
                                                   unique_ptr_expected);
}

TEST_F(CustomElementFactoryTest, createElement_successful)
{
    // It should compile and pass the correct arguments
    auto factory = fep3::core::CustomElementFactory<MyJobElement>(int_expected,
                                                                  std::ref(int_ref_expected),
                                                                  1,
                                                                  string_expected,
                                                                  std::ref(string_ref_expected),
                                                                  "string literal",
                                                                  shared_ptr_expected,
                                                                  unique_ptr_expected);

    std::unique_ptr<fep3::base::IElement> element = factory.createElement(*components);
    bool is_default_job_element = true;

    if (dynamic_cast<fep3::core::DefaultJobElement<MyJobElement>*>(element.get()) == nullptr) {
        is_default_job_element = false;
    }

    ASSERT_TRUE(is_default_job_element);
}
