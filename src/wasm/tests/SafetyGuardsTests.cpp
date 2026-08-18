#include <gtest/gtest.h>

#include <limits>

#include "SafetyGuards.h"

using e195e2::safety::RateLimit;
using e195e2::safety::SafeDivide;
using e195e2::safety::SanitizeDeltaTime;
using e195e2::safety::SanitizeDouble;
using e195e2::safety::SanitizeDoubleStrict;

TEST(SanitizeDouble, PassesThroughValueWithinRange) {
    EXPECT_DOUBLE_EQ(SanitizeDouble(50.0, 0.0, 100.0, -1.0), 50.0);
}

TEST(SanitizeDouble, ClampsBelowMin) {
    EXPECT_DOUBLE_EQ(SanitizeDouble(-10.0, 0.0, 100.0, -1.0), 0.0);
}

TEST(SanitizeDouble, ClampsAboveMax) {
    EXPECT_DOUBLE_EQ(SanitizeDouble(150.0, 0.0, 100.0, -1.0), 100.0);
}

TEST(SanitizeDouble, ReturnsFallbackOnNaN) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    EXPECT_DOUBLE_EQ(SanitizeDouble(nan, 0.0, 100.0, -1.0), -1.0);
}

TEST(SanitizeDouble, ReturnsFallbackOnInfinity) {
    const double inf = std::numeric_limits<double>::infinity();
    EXPECT_DOUBLE_EQ(SanitizeDouble(inf, 0.0, 100.0, -1.0), -1.0);
    EXPECT_DOUBLE_EQ(SanitizeDouble(-inf, 0.0, 100.0, -1.0), -1.0);
}

TEST(SanitizeDouble, ReturnsFallbackOnInvalidRange) {
    // min > max: faixa mal configurada pelo chamador.
    EXPECT_DOUBLE_EQ(SanitizeDouble(50.0, 100.0, 0.0, -1.0), -1.0);
}

TEST(SanitizeDoubleStrict, OutOfRangeReturnsFallbackInsteadOfClamping) {
    EXPECT_DOUBLE_EQ(SanitizeDoubleStrict(150.0, 0.0, 100.0, -1.0), -1.0);
    EXPECT_DOUBLE_EQ(SanitizeDoubleStrict(50.0, 0.0, 100.0, -1.0), 50.0);
}

TEST(SafeDivide, NormalDivision) {
    EXPECT_DOUBLE_EQ(SafeDivide(10.0, 2.0, -1.0), 5.0);
}

TEST(SafeDivide, DivisionByNearZeroReturnsFallback) {
    EXPECT_DOUBLE_EQ(SafeDivide(10.0, 1e-12, -1.0), -1.0);
}

TEST(SafeDivide, NanOperandReturnsFallback) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    EXPECT_DOUBLE_EQ(SafeDivide(nan, 2.0, -1.0), -1.0);
}

TEST(RateLimit, ClampsUpwardDelta) {
    // De 0 para 100 em 1s, com taxa máxima de 10/s -> só pode avançar 10.
    EXPECT_DOUBLE_EQ(RateLimit(0.0, 100.0, 10.0, 1.0), 10.0);
}

TEST(RateLimit, ClampsDownwardDelta) {
    EXPECT_DOUBLE_EQ(RateLimit(100.0, 0.0, 10.0, 1.0), 90.0);
}

TEST(RateLimit, PassesThroughSmallDelta) {
    EXPECT_DOUBLE_EQ(RateLimit(50.0, 55.0, 100.0, 1.0), 55.0);
}

TEST(RateLimit, InvalidDtReturnsPreviousValue) {
    EXPECT_DOUBLE_EQ(RateLimit(50.0, 100.0, 10.0, -1.0), 50.0);
    EXPECT_DOUBLE_EQ(RateLimit(50.0, 100.0, 10.0, 0.0), 50.0);
}

TEST(SanitizeDeltaTime, PassesThroughNormalValue) {
    EXPECT_DOUBLE_EQ(SanitizeDeltaTime(1.0 / 60.0), 1.0 / 60.0);
}

TEST(SanitizeDeltaTime, NegativeOrZeroReturnsFallback) {
    EXPECT_DOUBLE_EQ(SanitizeDeltaTime(0.0, 0.5), 0.5);
    EXPECT_DOUBLE_EQ(SanitizeDeltaTime(-1.0, 0.5), 0.5);
}

TEST(SanitizeDeltaTime, CapsExcessivelyLargeDt) {
    EXPECT_DOUBLE_EQ(SanitizeDeltaTime(10.0, 1.0 / 60.0, 0.25), 0.25);
}
