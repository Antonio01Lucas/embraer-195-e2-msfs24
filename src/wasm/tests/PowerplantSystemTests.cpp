#include <gtest/gtest.h>

#include "PowerplantSystem.h"

using namespace e195e2::systems;

namespace {
void runFor(PowerplantSystem& engine, double totalSeconds, double dt = 0.1) {
    for (double t = 0.0; t < totalSeconds; t += dt) {
        engine.update(dt);
    }
}
}

TEST(PowerplantSystem, StartsNotRunningWithZeroThrust) {
    PowerplantSystem engine(1);
    engine.update(1.0 / 60.0);
    EXPECT_FALSE(engine.isRunning());
    EXPECT_DOUBLE_EQ(engine.netThrustNewtons(), 0.0);
}

TEST(PowerplantSystem, FullThrottleConvergesTowardMaxThrust) {
    PowerplantSystem engine(1);
    engine.setThrottleInput(1.0);
    runFor(engine, 15.0);

    EXPECT_TRUE(engine.isRunning());
    EXPECT_GT(engine.n1Percent(), 95.0);
    // Ref: dados-tecnicos-extraidos.md - variante de maior empuxo do
    // PW1900G (~104.500 N / ~23.500 lbf).
    EXPECT_GT(engine.netThrustNewtons(), 0.9 * 104'500.0);
}

TEST(PowerplantSystem, N1NeverExceedsSanitizedCeiling) {
    PowerplantSystem engine(2);
    engine.setThrottleInput(1.0);
    runFor(engine, 30.0);
    EXPECT_LE(engine.n1Percent(), 110.0);
}

TEST(PowerplantSystem, IdleThrottleDecaysThrustBackToZero) {
    PowerplantSystem engine(1);
    engine.setThrottleInput(1.0);
    runFor(engine, 15.0);
    ASSERT_GT(engine.netThrustNewtons(), 0.0);

    engine.setThrottleInput(0.0);
    runFor(engine, 15.0);
    EXPECT_NEAR(engine.netThrustNewtons(), 0.0, 1.0);
}

TEST(PowerplantSystem, NgApproximatesN2ForThisTwoSpoolArchitecture) {
    PowerplantSystem engine(1);
    engine.setThrottleInput(0.7);
    runFor(engine, 15.0);
    EXPECT_DOUBLE_EQ(engine.ngPercent(), engine.n2Percent());
}
