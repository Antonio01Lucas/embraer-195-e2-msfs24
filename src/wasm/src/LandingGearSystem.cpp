#include "LandingGearSystem.h"

// Implementação mínima: transição de trem "instantânea" (snap) entre
// Up/Down — sequenciamento gradual real (portas, InTransit temporizado) é
// TODO(Fase 1), ver LandingGearSystem.h, para não exigir novos campos de
// estado além dos já declarados no cabeçalho já publicado.

namespace e195e2::systems {

namespace {
constexpr double kSteeringRatePerSecond = 120.0; // deg/s - estimativa genérica
}

void LandingGearSystem::update(double dt) noexcept {
    const double safeDt = safety::SanitizeDeltaTime(dt);
    recomputeGearState(safeDt);
}

void LandingGearSystem::setGearLeverDown(bool down) noexcept {
    gearLeverDown_ = down;
}

void LandingGearSystem::requestEmergencyGravityExtension() noexcept {
    emergencyExtensionRequested_ = true;
}

void LandingGearSystem::setAutobrakeMode(AutobrakeMode mode) noexcept {
    autobrakeMode_ = mode;
}

void LandingGearSystem::setManualBrakeInput(double leftInput, double rightInput) noexcept {
    manualBrakeLeftInput_ = safety::SanitizeDouble(leftInput, 0.0, 1.0, 0.0);
    manualBrakeRightInput_ = safety::SanitizeDouble(rightInput, 0.0, 1.0, 0.0);
}

void LandingGearSystem::setNoseWheelSteeringInputDegrees(double degrees) noexcept {
    noseWheelSteeringInputDegrees_ =
        safety::SanitizeDouble(degrees, -kMaxNoseSteeringDegrees, kMaxNoseSteeringDegrees, 0.0);
}

void LandingGearSystem::setHydraulicPressureAvailablePsi(double system1Psi, double system2Psi) noexcept {
    hydraulicPressureSystem1Psi_ = safety::SanitizeDouble(system1Psi, 0.0, 3200.0, 0.0);
    hydraulicPressureSystem2Psi_ = safety::SanitizeDouble(system2Psi, 0.0, 3200.0, 0.0);
}

GearPosition LandingGearSystem::nosePosition() const noexcept { return noseGear_; }
GearPosition LandingGearSystem::leftMainPosition() const noexcept { return leftMainGear_; }
GearPosition LandingGearSystem::rightMainPosition() const noexcept { return rightMainGear_; }
bool LandingGearSystem::isAntiSkidActive() const noexcept { return antiSkidActive_; }
bool LandingGearSystem::isEmergencyExtensionUsed() const noexcept { return emergencyExtensionUsed_; }
double LandingGearSystem::noseWheelSteeringAngleDegrees() const noexcept { return noseWheelSteeringAngleDegrees_; }
AutobrakeMode LandingGearSystem::autobrakeMode() const noexcept { return autobrakeMode_; }

void LandingGearSystem::recomputeGearState(double dt) noexcept {
    if (emergencyExtensionRequested_) {
        // Extensão por gravidade bypassa o sistema hidráulico normal —
        // TODO(Fase 1): tempo de extensão real e travamento mecânico.
        noseGear_ = GearPosition::Down;
        leftMainGear_ = GearPosition::Down;
        rightMainGear_ = GearPosition::Down;
        emergencyExtensionUsed_ = true;
    } else {
        const GearPosition target = gearLeverDown_ ? GearPosition::Down : GearPosition::Up;
        noseGear_ = target;
        leftMainGear_ = target;
        rightMainGear_ = target;
    }

    antiSkidActive_ = true; // TODO(Fase 1): desativar sob falha de sensor simulada

    noseWheelSteeringAngleDegrees_ = safety::RateLimit(
        noseWheelSteeringAngleDegrees_,
        noseWheelSteeringInputDegrees_,
        kSteeringRatePerSecond,
        dt
    );
}

} // namespace e195e2::systems
