#include "PneumaticSystem.h"

// Implementação mínima: pressão/temperatura de duto convergem para o
// nominal via rate-limit quando a sangria correspondente está aberta.
// Acoplamento real com PowerplantSystem (sangria depende de N2) e com
// APUSystem permanece TODO(Fase 1) — ver PneumaticSystem.h.

namespace e195e2::systems {

namespace {
constexpr double kPressureRatePerSecond = 15.0; // psi/s - estimativa genérica
constexpr double kTempRatePerSecondCelsius = 40.0; // °C/s - estimativa genérica
}

void PneumaticSystem::update(double dt) noexcept {
    const double safeDt = safety::SanitizeDeltaTime(dt);
    recomputeDuctState(safeDt);
}

void PneumaticSystem::setEngineBleedValveOpen(int engineIndex, bool open) noexcept {
    if (engineIndex == 1) engine1BleedOpen_ = open;
    else if (engineIndex == 2) engine2BleedOpen_ = open;
}

void PneumaticSystem::setApuBleedValveOpen(bool open) noexcept {
    apuBleedOpen_ = open;
}

void PneumaticSystem::setCrossbleedValveOpen(bool open) noexcept {
    crossbleedValveOpen_ = open;
}

void PneumaticSystem::setGroundCartConnected(bool connected) noexcept {
    groundCartConnected_ = connected;
}

void PneumaticSystem::setIsolationValveOpen(bool open) noexcept {
    isolationValveOpen_ = open;
}

double PneumaticSystem::ductPressurePsi(int side) const noexcept {
    return side == 1 ? ductPressureLeftPsi_ : ductPressureRightPsi_;
}

double PneumaticSystem::ductTemperatureCelsius(int side) const noexcept {
    return side == 1 ? ductTempLeftCelsius_ : ductTempRightCelsius_;
}

bool PneumaticSystem::isCrossbleedOpen() const noexcept {
    return crossbleedValveOpen_;
}

bool PneumaticSystem::isGroundCartSupplying() const noexcept {
    return groundCartConnected_;
}

void PneumaticSystem::recomputeDuctState(double dt) noexcept {
    const bool leftSupplied = engine1BleedOpen_ || groundCartConnected_ ||
        (crossbleedValveOpen_ && (engine2BleedOpen_ || apuBleedOpen_));
    const bool rightSupplied = engine2BleedOpen_ || apuBleedOpen_ ||
        (crossbleedValveOpen_ && engine1BleedOpen_);

    const double targetPressureLeft = (isolationValveOpen_ && leftSupplied) ? kNominalDuctPressurePsi : 0.0;
    const double targetPressureRight = (isolationValveOpen_ && rightSupplied) ? kNominalDuctPressurePsi : 0.0;
    const double targetTempLeft = leftSupplied ? kNominalDuctTempCelsius : 15.0;
    const double targetTempRight = rightSupplied ? kNominalDuctTempCelsius : 15.0;

    ductPressureLeftPsi_ = safety::RateLimit(ductPressureLeftPsi_, targetPressureLeft, kPressureRatePerSecond, dt);
    ductPressureRightPsi_ = safety::RateLimit(ductPressureRightPsi_, targetPressureRight, kPressureRatePerSecond, dt);
    ductTempLeftCelsius_ = safety::RateLimit(ductTempLeftCelsius_, targetTempLeft, kTempRatePerSecondCelsius, dt);
    ductTempRightCelsius_ = safety::RateLimit(ductTempRightCelsius_, targetTempRight, kTempRatePerSecondCelsius, dt);

    ductPressureLeftPsi_ = safety::SanitizeDouble(ductPressureLeftPsi_, 0.0, kMaxDuctPressurePsi, 0.0);
    ductPressureRightPsi_ = safety::SanitizeDouble(ductPressureRightPsi_, 0.0, kMaxDuctPressurePsi, 0.0);
    ductTempLeftCelsius_ = safety::SanitizeDouble(ductTempLeftCelsius_, -60.0, kMaxDuctTempCelsius, 15.0);
    ductTempRightCelsius_ = safety::SanitizeDouble(ductTempRightCelsius_, -60.0, kMaxDuctTempCelsius, 15.0);
}

} // namespace e195e2::systems
