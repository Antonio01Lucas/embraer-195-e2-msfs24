#include <gtest/gtest.h>

#include "HydraulicSystem.h"

using namespace e195e2::systems;

namespace {
void runFor(HydraulicSystem& hyd, double totalSeconds, double dt = 0.1) {
    for (double t = 0.0; t < totalSeconds; t += dt) {
        hyd.update(dt);
    }
}
}

TEST(HydraulicSystem, StartsDepressurized) {
    HydraulicSystem hyd;
    hyd.update(1.0 / 60.0);
    EXPECT_FALSE(hyd.isPressurized(HydraulicSystemId::System1));
    EXPECT_FALSE(hyd.isPressurized(HydraulicSystemId::System2));
    EXPECT_FALSE(hyd.isPressurized(HydraulicSystemId::System3));
}

TEST(HydraulicSystem, EnginePumpPressurizesSystemToNominal) {
    HydraulicSystem hyd;
    hyd.setEnginePumpActive(HydraulicSystemId::System1, true);
    runFor(hyd, 10.0);

    EXPECT_TRUE(hyd.isPressurized(HydraulicSystemId::System1));
    EXPECT_NEAR(hyd.pressurePsi(HydraulicSystemId::System1), 3000.0, 5.0);
}

TEST(HydraulicSystem, PressureNeverExceedsTransientCeiling) {
    HydraulicSystem hyd;
    hyd.setEnginePumpActive(HydraulicSystemId::System2, true);
    runFor(hyd, 30.0);
    EXPECT_LE(hyd.pressurePsi(HydraulicSystemId::System2), 3200.0);
}

TEST(HydraulicSystem, PtuTransfersPartialPressureWhenOneSideHasNoPump) {
    HydraulicSystem hyd;
    hyd.setEnginePumpActive(HydraulicSystemId::System2, true);
    hyd.setPtuActive(true);
    runFor(hyd, 10.0);

    // Sistema 1 não tem bomba própria ativa, mas PTU deve transferir
    // pressão parcial a partir do Sistema 2 pressurizado.
    EXPECT_GT(hyd.pressurePsi(HydraulicSystemId::System1), 0.0);
    EXPECT_LT(hyd.pressurePsi(HydraulicSystemId::System1), hyd.pressurePsi(HydraulicSystemId::System2));
}

TEST(HydraulicSystem, StoppingPumpDepressurizesOverTime) {
    HydraulicSystem hyd;
    hyd.setEnginePumpActive(HydraulicSystemId::System1, true);
    runFor(hyd, 10.0);
    ASSERT_TRUE(hyd.isPressurized(HydraulicSystemId::System1));

    hyd.setEnginePumpActive(HydraulicSystemId::System1, false);
    runFor(hyd, 10.0);
    EXPECT_FALSE(hyd.isPressurized(HydraulicSystemId::System1));
}
