#include <gtest/gtest.h>

#include "PneumaticSystem.h"

using namespace e195e2::systems;

namespace {
void runFor(PneumaticSystem& sys, double totalSeconds, double dt = 0.1) {
    for (double t = 0.0; t < totalSeconds; t += dt) {
        sys.update(dt);
    }
}
}

TEST(PneumaticSystem, StartsWithNoDuctPressure) {
    PneumaticSystem pneu;
    pneu.update(1.0 / 60.0);
    EXPECT_DOUBLE_EQ(pneu.ductPressurePsi(1), 0.0);
    EXPECT_DOUBLE_EQ(pneu.ductPressurePsi(2), 0.0);
}

TEST(PneumaticSystem, EngineBleedPressurizesCorrespondingSide) {
    PneumaticSystem pneu;
    pneu.setEngineBleedValveOpen(1, true);
    runFor(pneu, 5.0);

    EXPECT_GT(pneu.ductPressurePsi(1), 0.0);
    EXPECT_DOUBLE_EQ(pneu.ductPressurePsi(2), 0.0);
}

TEST(PneumaticSystem, CrossbleedSuppliesOppositeSide) {
    PneumaticSystem pneu;
    pneu.setEngineBleedValveOpen(1, true);
    pneu.setCrossbleedValveOpen(true);
    runFor(pneu, 5.0);

    EXPECT_TRUE(pneu.isCrossbleedOpen());
    EXPECT_GT(pneu.ductPressurePsi(2), 0.0);
}

TEST(PneumaticSystem, ClosingIsolationValveDepressurizesBothSides) {
    PneumaticSystem pneu;
    pneu.setEngineBleedValveOpen(1, true);
    pneu.setEngineBleedValveOpen(2, true);
    runFor(pneu, 5.0);
    ASSERT_GT(pneu.ductPressurePsi(1), 0.0);

    pneu.setIsolationValveOpen(false);
    runFor(pneu, 5.0);
    EXPECT_DOUBLE_EQ(pneu.ductPressurePsi(1), 0.0);
    EXPECT_DOUBLE_EQ(pneu.ductPressurePsi(2), 0.0);
}

TEST(PneumaticSystem, GroundCartSuppliesLeftSide) {
    PneumaticSystem pneu;
    EXPECT_FALSE(pneu.isGroundCartSupplying());
    pneu.setGroundCartConnected(true);
    runFor(pneu, 5.0);
    EXPECT_TRUE(pneu.isGroundCartSupplying());
    EXPECT_GT(pneu.ductPressurePsi(1), 0.0);
}
