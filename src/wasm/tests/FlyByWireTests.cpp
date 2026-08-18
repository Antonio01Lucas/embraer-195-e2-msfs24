#include <gtest/gtest.h>

#include "FlyByWire.h"

using namespace e195e2::systems;

namespace {
void runFor(FlyByWire& fbw, double totalSeconds, double dt = 0.1) {
    for (double t = 0.0; t < totalSeconds; t += dt) {
        fbw.update(dt);
    }
}
}

TEST(FlyByWire, DefaultsToNormalLawWithNoProtectionsActive) {
    FlyByWire fbw;
    fbw.update(1.0 / 60.0);
    EXPECT_EQ(fbw.activeControlLaw(), ControlLaw::Normal);
    EXPECT_FALSE(fbw.isHighAoaProtectionActive());
    EXPECT_FALSE(fbw.isBankLimitProtectionActive());
}

TEST(FlyByWire, ForceControlLawSwitchesToDirect) {
    FlyByWire fbw;
    fbw.forceControlLaw(ControlLaw::Direct);
    fbw.update(1.0 / 60.0);
    EXPECT_EQ(fbw.activeControlLaw(), ControlLaw::Direct);
}

TEST(FlyByWire, FullPitchCommandDeflectsElevatorTowardCommandedSide) {
    FlyByWire fbw;
    fbw.setPitchCommand(1.0);
    runFor(fbw, 5.0);
    EXPECT_GT(fbw.surfacePositions().elevatorDegrees, 15.0);
}

TEST(FlyByWire, HighAoaProtectionSuppressesNoseUpCommand) {
    FlyByWire fbw;
    fbw.setPitchCommand(1.0);
    fbw.setCurrentAngleOfAttackDegrees(20.0); // acima do limiar (14°, ver header)
    runFor(fbw, 5.0);

    EXPECT_TRUE(fbw.isHighAoaProtectionActive());
    EXPECT_LE(fbw.surfacePositions().elevatorDegrees, 0.0);
}

TEST(FlyByWire, BankLimitProtectionActivatesBeyondThreshold) {
    FlyByWire fbw;
    fbw.setCurrentBankAngleDegrees(80.0); // acima do limiar (67°, ver header)
    fbw.update(1.0 / 60.0);
    EXPECT_TRUE(fbw.isBankLimitProtectionActive());
}

TEST(FlyByWire, ProtectionsDoNotActivateInDirectLaw) {
    FlyByWire fbw;
    fbw.forceControlLaw(ControlLaw::Direct);
    fbw.setCurrentAngleOfAttackDegrees(20.0);
    fbw.setCurrentBankAngleDegrees(80.0);
    fbw.update(1.0 / 60.0);

    EXPECT_FALSE(fbw.isHighAoaProtectionActive());
    EXPECT_FALSE(fbw.isBankLimitProtectionActive());
}

TEST(FlyByWire, FlapLeverMovesFlapAndSlatTogether) {
    FlyByWire fbw;
    fbw.setFlapLeverPosition(5); // detente máximo -> 100%
    runFor(fbw, 30.0);

    EXPECT_NEAR(fbw.surfacePositions().flapPercent, 100.0, 1.0);
    EXPECT_DOUBLE_EQ(fbw.surfacePositions().slatPercent, fbw.surfacePositions().flapPercent);
}
