#include "SimVarPublisher.h"

namespace e195e2::systems {

double SimVarPublisher::gearPositionValue(GearPosition position) noexcept {
    switch (position) {
        case GearPosition::Up: return 0.0;
        case GearPosition::InTransit: return 1.0;
        case GearPosition::Down: return 2.0;
    }
    return 0.0;
}

double SimVarPublisher::controlLawValue(ControlLaw law) noexcept {
    return law == ControlLaw::Direct ? 1.0 : 0.0;
}

SimVarPublisher::Samples SimVarPublisher::collect(const SimBus& bus) noexcept {
    const auto& elec = bus.electrical();
    const auto& gear = bus.landingGear();
    const auto& fbw = bus.flyByWire();
    const auto& surfaces = fbw.surfacePositions();

    return Samples{{
        {"E195E2_ELEC_AC1_POWERED", "Bool", elec.isBusPowered(ElectricalBus::AC1) ? 1.0 : 0.0},
        {"E195E2_ELEC_AC2_POWERED", "Bool", elec.isBusPowered(ElectricalBus::AC2) ? 1.0 : 0.0},
        {"E195E2_ELEC_AC_ESSENTIAL_POWERED", "Bool", elec.isBusPowered(ElectricalBus::ACEssential) ? 1.0 : 0.0},
        {"E195E2_ELEC_DC1_POWERED", "Bool", elec.isBusPowered(ElectricalBus::DC1) ? 1.0 : 0.0},
        {"E195E2_ELEC_DC2_POWERED", "Bool", elec.isBusPowered(ElectricalBus::DC2) ? 1.0 : 0.0},
        {"E195E2_ELEC_DC_ESSENTIAL_POWERED", "Bool", elec.isBusPowered(ElectricalBus::DCEssential) ? 1.0 : 0.0},
        {"E195E2_ELEC_DC_EMERGENCY_POWERED", "Bool", elec.isBusPowered(ElectricalBus::DCEmergency) ? 1.0 : 0.0},
        {"E195E2_ELEC_LOAD_SHEDDING_ACTIVE", "Bool", elec.isLoadSheddingActive() ? 1.0 : 0.0},
        {"E195E2_ELEC_BATTERY_1_PERCENT", "percent", elec.batteryChargePercent(1)},
        {"E195E2_ELEC_BATTERY_2_PERCENT", "percent", elec.batteryChargePercent(2)},

        {"E195E2_GEAR_NOSE_POSITION", "number", gearPositionValue(gear.nosePosition())},
        {"E195E2_GEAR_LEFT_MAIN_POSITION", "number", gearPositionValue(gear.leftMainPosition())},
        {"E195E2_GEAR_RIGHT_MAIN_POSITION", "number", gearPositionValue(gear.rightMainPosition())},
        {"E195E2_GEAR_NOSEWHEEL_STEERING_DEG", "degrees", gear.noseWheelSteeringAngleDegrees()},

        {"E195E2_FBW_FLAP_PERCENT", "percent", surfaces.flapPercent},
        {"E195E2_FBW_SLAT_PERCENT", "percent", surfaces.slatPercent},
        {"E195E2_FBW_SPEEDBRAKE_PERCENT", "percent", surfaces.speedbrakePercent},
        {"E195E2_FBW_GROUND_SPOILER_PERCENT", "percent", surfaces.groundSpoilerPercent},
        {"E195E2_FBW_ROLL_SPOILER_PERCENT", "percent", surfaces.rollSpoilerPercent},
        {"E195E2_FBW_CONTROL_LAW", "number", controlLawValue(fbw.activeControlLaw())},
    }};
}

} // namespace e195e2::systems
