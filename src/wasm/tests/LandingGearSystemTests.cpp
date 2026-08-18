#include <gtest/gtest.h>

#include "LandingGearSystem.h"

using namespace e195e2::systems;

TEST(LandingGearSystem, DefaultsToGearDown) {
    LandingGearSystem gear;
    gear.update(1.0 / 60.0);
    EXPECT_EQ(gear.nosePosition(), GearPosition::Down);
    EXPECT_EQ(gear.leftMainPosition(), GearPosition::Down);
    EXPECT_EQ(gear.rightMainPosition(), GearPosition::Down);
}

TEST(LandingGearSystem, RaisingLeverRetractsGear) {
    LandingGearSystem gear;
    gear.setGearLeverDown(false);
    gear.update(1.0 / 60.0);
    EXPECT_EQ(gear.nosePosition(), GearPosition::Up);
    EXPECT_EQ(gear.leftMainPosition(), GearPosition::Up);
    EXPECT_EQ(gear.rightMainPosition(), GearPosition::Up);
}

TEST(LandingGearSystem, EmergencyExtensionForcesGearDownRegardlessOfLever) {
    LandingGearSystem gear;
    gear.setGearLeverDown(false);
    gear.update(1.0 / 60.0);
    ASSERT_EQ(gear.nosePosition(), GearPosition::Up);

    gear.requestEmergencyGravityExtension();
    gear.update(1.0 / 60.0);
    EXPECT_EQ(gear.nosePosition(), GearPosition::Down);
    EXPECT_TRUE(gear.isEmergencyExtensionUsed());
}

TEST(LandingGearSystem, SteeringAngleRateLimitedTowardCommandedInput) {
    LandingGearSystem gear;
    gear.setNoseWheelSteeringInputDegrees(78.0);
    // Um único frame pequeno não deve saltar instantaneamente para o alvo.
    gear.update(1.0 / 60.0);
    EXPECT_LT(gear.noseWheelSteeringAngleDegrees(), 78.0);

    // Após tempo suficiente, deve convergir para o alvo comandado.
    for (int i = 0; i < 100; ++i) {
        gear.update(0.1);
    }
    EXPECT_NEAR(gear.noseWheelSteeringAngleDegrees(), 78.0, 0.5);
}

TEST(LandingGearSystem, AutobrakeModeReflectsSetter) {
    LandingGearSystem gear;
    EXPECT_EQ(gear.autobrakeMode(), AutobrakeMode::Off);
    gear.setAutobrakeMode(AutobrakeMode::Rto);
    EXPECT_EQ(gear.autobrakeMode(), AutobrakeMode::Rto);
}
