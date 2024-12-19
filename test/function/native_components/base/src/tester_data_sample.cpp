/**
 * Copyright 2023 CARIAD SE.
 *
 * This Source Code Form is subject to the terms of the Mozilla
 * Public License, v. 2.0. If a copy of the MPL was not distributed
 * with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <fep3/base/sample/data_sample.h>

#include <gtest/gtest.h>

/**
 * @detail Test the timestamp and counter functionality of class fep3::base::DataSample
 * @req_id FEPSDK-Sample
 */
TEST(DataSampleTest, testTimestampAndCounter)
{
    { // default construction
        fep3::base::DataSample sample;
        EXPECT_EQ(0u, sample.getCounter());
        EXPECT_EQ(fep3::Timestamp(0), sample.getTime());
    }

    { // construction with counter value and timestamp
        fep3::base::RawMemoryRef memory(nullptr, 0);
        fep3::base::DataSample sample(fep3::Timestamp(33), 44, memory);
        EXPECT_EQ(fep3::Timestamp(33), sample.getTime());
        EXPECT_EQ(44u, sample.getCounter());
    }

    { // copy construction
        fep3::base::DataSample sample_1;
        sample_1.setTime(fep3::Timestamp(33));
        sample_1.setCounter(44);
        fep3::base::DataSample sample_2(sample_1);
        EXPECT_EQ(fep3::Timestamp(33), sample_2.getTime());
        EXPECT_EQ(44u, sample_2.getCounter());
    }

    { // move construction
        fep3::base::DataSample sample_1;
        sample_1.setTime(fep3::Timestamp(33));
        sample_1.setCounter(44);
        fep3::base::DataSample sample_2(std::move(sample_1));
        EXPECT_EQ(fep3::Timestamp(33), sample_2.getTime());
        EXPECT_EQ(44u, sample_2.getCounter());
    }

    { // copy assignment
        fep3::base::DataSample sample_1;
        sample_1.setTime(fep3::Timestamp(33));
        sample_1.setCounter(44);
        fep3::base::DataSample sample_2;
        sample_2 = sample_1;
        EXPECT_EQ(fep3::Timestamp(33), sample_2.getTime());
        EXPECT_EQ(44u, sample_2.getCounter());
    }

    { // move assignment
        fep3::base::DataSample sample_1;
        sample_1.setTime(fep3::Timestamp(33));
        sample_1.setCounter(44);
        fep3::base::DataSample sample_2;
        sample_2 = std::move(sample_1);
        EXPECT_EQ(fep3::Timestamp(33), sample_2.getTime());
        EXPECT_EQ(44u, sample_2.getCounter());
    }
}

template <typename data_sample_class>
class DataSampleTypeTest : public ::testing::Test {
};

class MyClass {
};
using DataSampleTypeTypes = ::testing::Types<int, MyClass>;
TYPED_TEST_SUITE(DataSampleTypeTest, DataSampleTypeTypes);

/**
 * @detail Test the counter and time functionality of class fep3::base::DataSampleType<int>
 * @req_id FEPSDK-Sample
 */
TYPED_TEST(DataSampleTypeTest, testCounterAndTime)
{
    { // construction
        TypeParam value;
        fep3::base::DataSampleType<TypeParam> sample(value);
        EXPECT_EQ(0u, sample.getCounter());
        EXPECT_EQ(fep3::Timestamp(0), sample.getTime());
    }
}

/**
 * @detail Test the copying of StdVectorSampleType through IDataSample
 * @req_id FEPSDK-Sample
 */
TEST(StdVectorSampleTypeTest, testCopy)
{
    using namespace std::literals::chrono_literals;
    const size_t element_count = 10;

    struct TestVector {
        bool valid;
        int x;
        double length;
        float y;
        char padding;
    };

    auto compare_two_test_vectors = [](const TestVector& l, const TestVector& r) {
        bool is_equal = l.valid == r.valid;
        is_equal &= l.x == r.x;
        is_equal &= l.length == r.length;
        is_equal &= l.y == r.y;
        is_equal &= l.padding == r.padding;
        return is_equal;
    };

    // Prepare std vector
    std::vector<TestVector> my_data;
    my_data.reserve(element_count);

    for (int i = element_count; i > 0; i--) {
        TestVector v{
            i % 2 ? true : false, i, i * 1.24, static_cast<float>(i) / 1.24f, static_cast<char>(i)};
        my_data.emplace_back(v);
    }

    // Prepare sample
    fep3::base::StdVectorSampleType<TestVector> array_sample{my_data};
    fep3::IDataSample* intf_sample = static_cast<fep3::IDataSample*>(&array_sample);

    // Copy sample via IDataSample interface
    fep3::base::DataSample sample_raw_copy{*intf_sample};

    std::vector<TestVector> my_copied_data;
    fep3::base::StdVectorSampleType<TestVector> copied_array_sample{my_copied_data};

    // Test if sample is equal to its copy with IRawMemory interface
    copied_array_sample.write(sample_raw_copy);
    ASSERT_TRUE(std::equal(
        my_data.begin(), my_data.end(), my_copied_data.begin(), compare_two_test_vectors));
    ASSERT_EQ(array_sample.getSize(), copied_array_sample.getSize());
}
