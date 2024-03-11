#include <gtest/gtest.h>
#include <bigint.hpp>
#include <cstdint>

TEST(SuiteTest, TestFortyTwo) {
  EXPECT_EQ(7 * 6, 42);
}

// When abs(val) < INT64_MAX

TEST(ConstructionTests, TestOneBlockStringPositive) {
  EXPECT_EQ(BigInt(73), "73"_bi);
  EXPECT_EQ(BigInt(73), BigInt("73"));
}

TEST(ConstructionTests, LeadingZero) {
  EXPECT_EQ(BigInt(73), "00073"_bi);
}

TEST(ConstructionTests, TestOneBlockStringZero) {
  EXPECT_EQ(BigInt(0), "0"_bi);
  EXPECT_EQ(BigInt(0), BigInt("0"));
}

TEST(ConstructionTests, TestOneBlockStringMultipleZero) {
  EXPECT_EQ(BigInt(0), "0000"_bi);
  EXPECT_EQ(BigInt(0), BigInt("00000"));
}

TEST(ConstructionTests, TestOneBlockStringNegativeZero) {
  EXPECT_EQ(BigInt(0), "-0000"_bi);
  EXPECT_EQ(BigInt(0), BigInt("-0000"));
}

TEST(ConstructionTests, TestOneBlockStringNegative) {
  EXPECT_EQ(BigInt(-335), "-335"_bi);
  EXPECT_EQ(BigInt(-335), BigInt("-335"));
}

TEST(MathTests, AddSmallPositive) {
  EXPECT_EQ(BigInt(23), "20"_bi + "3"_bi);
  EXPECT_EQ(BigInt(440), "420"_bi + "20"_bi);
}

TEST(MathTests, AddBecameNonZero) {
  BigInt a = "0"_bi;
  a += "23"_bi;
  EXPECT_EQ(BigInt(23), a);
}

TEST(MathTests, AddSmallNegative) {
  EXPECT_EQ(BigInt(-440), "-420"_bi + "-20"_bi);
}

TEST(MathTests, AddSmallOverflow) {
  BigInt expected_res = "8589934591"_bi;
  BigInt a = "4294967295"_bi;
  a += a;
  a += BigInt(1);

  EXPECT_EQ(expected_res, a);

  expected_res = "36893488147419103231"_bi;
  a = "18446744073709551615"_bi;
  a += a;
  a += BigInt(1);
  EXPECT_EQ(expected_res, a);
}

TEST(MathTests, AddSameBig) {
  BigInt expected_res =
      "12345678910111213141516171819201234567891011121314151617181920"_bi;
  BigInt a =
      "10305070900111210001516171819200000000000011121314151617181920"_bi;
  BigInt b =
      "02040608010000003140000000000001234567891000000000000000000000"_bi;

  EXPECT_EQ(expected_res, a + b);
}

TEST(MathTests, AddSmallToBig) {
  BigInt expected_res =
      "12345678910111213141516171819201234567891011121314151617181920"_bi;
  BigInt a =
      "12345678910111213141516171819201234567891011121314151617181919"_bi;
  BigInt b = "1"_bi;

  EXPECT_EQ(expected_res, a + b);
}

TEST(MathTests, AddBigToSmall) {
  BigInt expected_res =
      "12345678910111213141516171819201234567891011121314151617181920"_bi;
  BigInt a = "1"_bi;
  BigInt b =
      "12345678910111213141516171819201234567891011121314151617181919"_bi;

  EXPECT_EQ(expected_res, a + b);
}

TEST(MathTests, AddDangerousCase) {
  BigInt a = "18446744073709551615"_bi;  // UINT64_MAX
  BigInt b = a;

  EXPECT_EQ("36893488147419103230"_bi, a + b);
}

TEST(MathTests, AddIntEasyPositive) {
  EXPECT_EQ(BigInt(774), BigInt(700) + 74);
}

TEST(MathTests, AddIntEasyNegative) {
  EXPECT_EQ(BigInt(-774), BigInt(-700) + (-74));
}

TEST(MathTests, AddIntOverflow) {
  EXPECT_EQ(BigInt("4294967395"), "4294967295"_bi + 100);
}

TEST(MathTests, AddIntBecameNonZero) {
  EXPECT_EQ(BigInt("5"), "0"_bi + 5);
}

TEST(MathTests, MullIntEasyPositive) {
  EXPECT_EQ(BigInt("100"), "10"_bi * 10);
}

TEST(MathTests, MullIntEasyNegativeBI) {
  EXPECT_EQ(BigInt("-100"), "-10"_bi * 10);
}

TEST(MathTests, MullIntEasyNegativeInt) {
  EXPECT_EQ(BigInt("-100"), "10"_bi * (-10));
}

TEST(MathTests, MullIntOveflow) {
  EXPECT_EQ(BigInt("429496739000"), "429496739"_bi * (1000));
}

TEST(MathTests, MulIntZeroBI) {
  EXPECT_EQ(BigInt("0"), "0"_bi * 5);
}

TEST(MathTests, MulIntZeroInt) {
  EXPECT_EQ(BigInt("0"), "5"_bi * 0);
}

TEST(MathTests, SubIntZeroInt) {
  EXPECT_EQ(BigInt("5"), "5"_bi - 0);
}

TEST(MathTests, SubNegativities) {
  EXPECT_EQ(BigInt("2"), "-2"_bi - (-4));
}

TEST(MathTests, SubIntZeroBi) {
  EXPECT_EQ(BigInt("-5"), "0"_bi - 5);
}

TEST(MathTests, SubIntDownFlow) {
  EXPECT_EQ(BigInt("4294967195"), "4294967295"_bi - 100);
}

TEST(MathTests, SubIntBecomeZero) {
  EXPECT_EQ(BigInt("0"), "100"_bi - 100);
}

TEST(MathTests, SubIntBecomeZeroNegative) {
  EXPECT_EQ(BigInt("0"), "-100"_bi - (-100));
}

TEST(MathTests, Inc) {
  auto x = "3"_bi;
  EXPECT_EQ(BigInt("3"), x++);
  EXPECT_EQ(BigInt("4"), x);
  EXPECT_EQ(BigInt("5"), ++x);
  EXPECT_EQ(BigInt("5"), x);
}

TEST(MathTests, Dec) {
  auto x = "5"_bi;
  EXPECT_EQ(BigInt("5"), x--);
  EXPECT_EQ(BigInt("4"), x);
  EXPECT_EQ(BigInt("3"), --x);
  EXPECT_EQ(BigInt("3"), x);
}

TEST(MathTests, SubNormalEasy) {
  EXPECT_EQ(BigInt("1000"), "3000"_bi -
                                "2000"_bi);
}

TEST(MathTests, SubNormal) {
  EXPECT_EQ(BigInt("1000"), "1234545454545124154251425142521436271789"_bi -
                                "1234545454545124154251425142521436270789"_bi);
}

TEST(MathTests, SubLong) {
  EXPECT_EQ(BigInt("57389571782346329846291274893264873256238476234"),
            "57389573016891784391415429144690015777674748023"_bi -
                "1234545454545124154251425142521436271789"_bi);
}

TEST(MathTests, SubLongNegative) {
  EXPECT_EQ(BigInt("-57389571782346329846291274893264873256238476234"),
            "-57389573016891784391415429144690015777674748023"_bi -
                "-1234545454545124154251425142521436271789"_bi);
}

TEST(MathTests, SubLongBecomeZero) {
  EXPECT_EQ(BigInt("0"),
            "57389573016891784391415429144690015777674748023"_bi -
                "57389573016891784391415429144690015777674748023"_bi);
}

TEST(MathTests, SubDiffSign) {
  EXPECT_EQ(BigInt("8"),
            "5"_bi -
                "-3"_bi);
}

TEST(MathTests, AddDiffSign) {
  EXPECT_EQ(BigInt("2"),
            "5"_bi +
                "-3"_bi);
}

TEST(MathTests, SubBecomeZero) {
  EXPECT_EQ(BigInt("0"), "1234545454545124154251425142521436271789"_bi -
                             "1234545454545124154251425142521436271789"_bi);
}

TEST(MathTests, SubChangeSign) {
  EXPECT_EQ(BigInt("-100"), "1234545454545124154251425142521436271789"_bi -
                                "1234545454545124154251425142521436271889"_bi);
}
