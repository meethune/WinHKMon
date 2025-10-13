#include <gtest/gtest.h>

// Sample test to verify Google Test framework is working
TEST(SampleTest, BasicAssertion) {
    EXPECT_EQ(1, 1);
}

TEST(SampleTest, TrueIsTrue) {
    EXPECT_TRUE(true);
}

TEST(SampleTest, FalseIsFalse) {
    EXPECT_FALSE(false);
}

