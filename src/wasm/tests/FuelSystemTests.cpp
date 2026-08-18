#include <gtest/gtest.h>

#include "FuelSystem.h"

using namespace e195e2::systems;

TEST(FuelSystem, StartsEmpty) {
    FuelSystem fuel;
    fuel.update(1.0 / 60.0);
    EXPECT_DOUBLE_EQ(fuel.totalFuelKg(), 0.0);
}

TEST(FuelSystem, SetTankQuantityClampsToMaxUsable) {
    FuelSystem fuel;
    fuel.setTankQuantityKg(FuelTank::Left, 999'999.0);
    fuel.update(1.0 / 60.0);
    // Máximo utilizável total é 13.690 kg (folheto Embraer) — um único
    // tanque não deveria exceder isso.
    EXPECT_LE(fuel.tankQuantityKg(FuelTank::Left), 13'690.0);
}

TEST(FuelSystem, SetTankQuantityClampsNegativeToZero) {
    FuelSystem fuel;
    fuel.setTankQuantityKg(FuelTank::Right, -500.0);
    fuel.update(1.0 / 60.0);
    EXPECT_DOUBLE_EQ(fuel.tankQuantityKg(FuelTank::Right), 0.0);
}

TEST(FuelSystem, TotalFuelSumsBothTanks) {
    FuelSystem fuel;
    fuel.setTankQuantityKg(FuelTank::Left, 3000.0);
    fuel.setTankQuantityKg(FuelTank::Right, 2500.0);
    fuel.update(1.0 / 60.0);
    EXPECT_DOUBLE_EQ(fuel.totalFuelKg(), 5500.0);
}

TEST(FuelSystem, ConsumeFuelDecreasesTankAndNeverGoesNegative) {
    FuelSystem fuel;
    fuel.setTankQuantityKg(FuelTank::Left, 100.0);
    fuel.consumeFuel(FuelTank::Left, 40.0);
    EXPECT_DOUBLE_EQ(fuel.tankQuantityKg(FuelTank::Left), 60.0);

    fuel.consumeFuel(FuelTank::Left, 1000.0);
    EXPECT_DOUBLE_EQ(fuel.tankQuantityKg(FuelTank::Left), 0.0);
}

TEST(FuelSystem, ImbalanceReflectsAbsoluteDifference) {
    FuelSystem fuel;
    fuel.setTankQuantityKg(FuelTank::Left, 3000.0);
    fuel.setTankQuantityKg(FuelTank::Right, 2500.0);
    EXPECT_DOUBLE_EQ(fuel.imbalanceKg(), 500.0);
}

TEST(FuelSystem, PumpAndCrossfeedStateReflectsSetters) {
    FuelSystem fuel;
    EXPECT_FALSE(fuel.isPumpActive(FuelTank::Left, /*primary=*/true));
    fuel.setPumpActive(FuelTank::Left, /*primary=*/true, /*active=*/true);
    EXPECT_TRUE(fuel.isPumpActive(FuelTank::Left, /*primary=*/true));

    EXPECT_FALSE(fuel.isCrossfeedOpen());
    fuel.setCrossfeedValveOpen(true);
    EXPECT_TRUE(fuel.isCrossfeedOpen());
}
