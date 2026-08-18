#include <string_view>

#include <gtest/gtest.h>

#include "SimVarPublisher.h"

using namespace e195e2::systems;

namespace {
void runFor(SimBus& bus, double totalSeconds, double dt = 0.1) {
    for (double t = 0.0; t < totalSeconds; t += dt) {
        bus.update(dt);
    }
}

double sampleValue(const SimVarPublisher::Samples& samples, const char* name) {
    for (const auto& sample : samples) {
        if (std::string_view(sample.name) == name) {
            return sample.value;
        }
    }
    ADD_FAILURE() << "L:Var não encontrado na coleta: " << name;
    return -1.0;
}
}

TEST(SimVarPublisher, DefaultStateMapsToExpectedValues) {
    SimBus bus;
    bus.update(1.0 / 60.0);
    const auto samples = SimVarPublisher::collect(bus);

    EXPECT_EQ(sampleValue(samples, "E195E2_ELEC_AC1_POWERED"), 0.0);
    EXPECT_EQ(sampleValue(samples, "E195E2_ELEC_LOAD_SHEDDING_ACTIVE"), 1.0);
    // GearPosition::Down é o default (ver LandingGearSystem.h) -> 2.0
    // (Up=0, InTransit=1, Down=2 - ver SimVarPublisher::gearPositionValue).
    EXPECT_EQ(sampleValue(samples, "E195E2_GEAR_NOSE_POSITION"), 2.0);
    EXPECT_EQ(sampleValue(samples, "E195E2_FBW_FLAP_PERCENT"), 0.0);
    EXPECT_EQ(sampleValue(samples, "E195E2_FBW_CONTROL_LAW"), 0.0); // Normal
}

TEST(SimVarPublisher, ElectricalBusPoweredThroughTheBusReflectsInSamples) {
    SimBus bus;
    bus.engine(1).setThrottleInput(1.0);
    bus.update(1.0 / 60.0);
    bus.electrical().setGeneratorOnline(GeneratorSource::Idg1, true);
    bus.update(1.0 / 60.0);

    const auto samples = SimVarPublisher::collect(bus);
    EXPECT_EQ(sampleValue(samples, "E195E2_ELEC_AC1_POWERED"), 1.0);
    EXPECT_EQ(sampleValue(samples, "E195E2_ELEC_LOAD_SHEDDING_ACTIVE"), 0.0);
}

TEST(SimVarPublisher, GearLeverUpReflectsInSamples) {
    SimBus bus;
    bus.landingGear().setGearLeverDown(false);
    bus.update(1.0 / 60.0);

    const auto samples = SimVarPublisher::collect(bus);
    EXPECT_EQ(sampleValue(samples, "E195E2_GEAR_NOSE_POSITION"), 0.0);
    EXPECT_EQ(sampleValue(samples, "E195E2_GEAR_LEFT_MAIN_POSITION"), 0.0);
    EXPECT_EQ(sampleValue(samples, "E195E2_GEAR_RIGHT_MAIN_POSITION"), 0.0);
}

TEST(SimVarPublisher, FlapLeverPositionReflectsInSamplesAfterActuatorTravel) {
    SimBus bus;
    bus.flyByWire().setFlapLeverPosition(3); // alvo: 3 * 20% = 60% (ver FlyByWire.cpp)
    // Taxa de 5%/s (kFlapRatePerSecondPercent) - 60% precisa de 12s;
    // margem para garantir convergência.
    runFor(bus, 15.0);

    const auto samples = SimVarPublisher::collect(bus);
    EXPECT_NEAR(sampleValue(samples, "E195E2_FBW_FLAP_PERCENT"), 60.0, 0.5);
    EXPECT_NEAR(sampleValue(samples, "E195E2_FBW_SLAT_PERCENT"), 60.0, 0.5);
}

TEST(SimVarPublisher, SpeedbrakeCommandReflectsInSamples) {
    SimBus bus;
    bus.flyByWire().setSpeedbrakeCommand(0.5);
    bus.update(1.0 / 60.0); // não é rate-limited (ver FlyByWire::recomputeSurfacePositions)

    const auto samples = SimVarPublisher::collect(bus);
    EXPECT_NEAR(sampleValue(samples, "E195E2_FBW_SPEEDBRAKE_PERCENT"), 50.0, 0.01);
}
