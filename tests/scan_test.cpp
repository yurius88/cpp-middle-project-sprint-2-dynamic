#include <gtest/gtest.h>
#include <list>
#include <print>

#include "scan.hpp"

TEST(ScanTest, HasErrorTest1) {
    auto result = stdx::scan<std::string>("number", "");
    ASSERT_TRUE(!result.has_value());
}

TEST(ScanTest, HasErrorTest2) {
    auto result = stdx::scan<std::string>("", "");
    ASSERT_TRUE(!result.has_value());
}

TEST(ScanTest, HasErrorTest3) {
    auto result = stdx::scan<std::string>("number", "{%d}");
    ASSERT_TRUE(!result.has_value());
}

TEST(ScanTest, HasErrorTest4) {
    auto result = stdx::scan<int>("number", "{%d}");
    ASSERT_TRUE(!result.has_value());
}

TEST(ScanTest, HasErrorTest5) {
    auto result = stdx::scan<double>("number", "{%f}");
    ASSERT_TRUE(!result.has_value());
}

TEST(ScanTest, HasErrorTest6) {
    auto result = stdx::scan<std::any>("I want to sum 42 and 3.14 numbers.", "I want to sum {%d} and 3.14 numbers.");
    ASSERT_TRUE(!result.has_value());
}

TEST(ScanTest, HasErrorTest7) {
    auto result =
        stdx::scan<std::vector<int>>("I want to sum 42 and 3.14 numbers.", "I want to sum {%d} and 3.14 numbers.");
    ASSERT_TRUE(!result.has_value());
}

TEST(ScanTest, HasErrorTest8) {
    auto result =
        stdx::scan<std::list<int>>("I want to sum 42 and 3.14 numbers.", "I want to sum {%d} and 3.14 numbers.");
    ASSERT_TRUE(!result.has_value());
}

TEST(ScanTest, HasDataTest1) {
    auto result = stdx::scan<int8_t>("I want to sum 42 and 3.14 numbers.", "I want to sum {%d} and 3.14 numbers.");
    ASSERT_TRUE(result.has_value());
}
TEST(ScanTest, HasDataTest2) {
    auto result =
        stdx::scan<int8_t, double>("I want to sum 42 and 3.14 numbers.", "I want to sum {%d} and {%f} numbers.");
    ASSERT_TRUE(result.has_value());
}

TEST(ScanTest, HasDataTest3) {
    auto result = stdx::scan<const int8_t, const double>("I want to sum 42 and 3.14 numbers.",
                                                         "I want to sum {%d} and {%f} numbers.");
    ASSERT_TRUE(result.has_value());
}

TEST(ScanTest, HasDataTest4) {
    auto result = stdx::scan<int8_t>("I want to sum 42 and 3.14 numbers.", "I want to sum {%d} and 3.14 numbers.");
    ASSERT_TRUE(result.has_value() && std::get<0>(result->values()) == 42);
}

TEST(ScanTest, HasDataTest5) {
    auto result = stdx::scan<float>("I want to sum 42 and 3.14 numbers.", "I want to sum 42 and {%f} numbers.");
    if (!result.has_value())
        FAIL();
    auto check_value = ((std::get<0>(result->values()) - 3.14f)) < 1e-15;
    ASSERT_TRUE(check_value);
}

TEST(ScanTest, HasDataTest6) {
    auto result = stdx::scan<std::string, std::string_view>("I want to sum 42 and 3.14 numbers.",
                                                            "I {%s} to sum 42 and 3.14 {%s}.");
    if (!result.has_value())
        FAIL();

    auto tuple_size = std::tuple_size_v<std::decay_t<decltype(result->values())>>;
    if (tuple_size != 2)
        FAIL();

    ASSERT_TRUE(std::get<0>(result->values()) == "want" && std::get<1>(result->values()) == "numbers");
}

TEST(ScanTest, HasDataTest7) {
    auto result = stdx::scan<int8_t, int16_t, uint32_t, uint64_t, float, double>("-42 -28 33 654545484545 12.5 -88.7",
                                                                                 "{%d} {%d} {%u} {%u} {%f} {%f}");
    ASSERT_TRUE(result.has_value());
}