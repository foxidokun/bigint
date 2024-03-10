#include <gtest/gtest.h>
#include <bigint.hpp>

TEST(SuiteTest, TestFortyTwo) {
  EXPECT_EQ(7 * 6, 42);
}

// When abs(val) < INT64_MAX

TEST(ConstructionTests, TestOneBlockStringPositive) {
  EXPECT_EQ(BigInt(73), BigInt("73"));
}

TEST(ConstructionTests, TestOneBlockStringZero) {
  EXPECT_EQ(BigInt(0), BigInt("0"));
}

TEST(ConstructionTests, TestOneBlockStringMultipleZero) {
  EXPECT_EQ(BigInt(0), BigInt("00000"));
}

TEST(ConstructionTests, TestOneBlockStringNegativeZero) {
  EXPECT_EQ(BigInt(0), BigInt("-0000"));
}

TEST(ConstructionTests, TestOneBlockStringNegative) {
  EXPECT_EQ(BigInt(-335), BigInt("-335"));
}