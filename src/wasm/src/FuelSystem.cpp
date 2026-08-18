#include "FuelSystem.h"

#include <cmath>

// Implementação mínima: quantidades por tanque com clamp de segurança.
// Balanceamento real por gravidade/pressão via crossfeed e sequenciamento
// primária/standby permanecem TODO(Fase 1) — ver FuelSystem.h.

namespace e195e2::systems {

void FuelSystem::update(double dt) noexcept {
    (void)safety::SanitizeDeltaTime(dt); // reservado para consumo dinâmico (Fase 1,
                                          // integrado com PowerplantSystem)
    clampTankQuantities();
}

void FuelSystem::setTankQuantityKg(FuelTank tank, double quantityKg) noexcept {
    const double sanitized = safety::SanitizeDouble(quantityKg, 0.0, kMaxUsableFuelKg, 0.0);
    if (tank == FuelTank::Left) {
        tankLeftKg_ = sanitized;
    } else {
        tankRightKg_ = sanitized;
    }
}

void FuelSystem::setPumpActive(FuelTank tank, bool primary, bool active) noexcept {
    if (tank == FuelTank::Left) {
        if (primary) pumpLeftPrimaryActive_ = active; else pumpLeftStandbyActive_ = active;
    } else {
        if (primary) pumpRightPrimaryActive_ = active; else pumpRightStandbyActive_ = active;
    }
}

void FuelSystem::setCrossfeedValveOpen(bool open) noexcept {
    crossfeedValveOpen_ = open;
}

void FuelSystem::consumeFuel(FuelTank tank, double massKg) noexcept {
    const double safeMass = safety::SanitizeDouble(massKg, 0.0, kMaxUsableFuelKg, 0.0);
    if (tank == FuelTank::Left) {
        tankLeftKg_ = safety::SanitizeDouble(tankLeftKg_ - safeMass, 0.0, kMaxUsableFuelKg, 0.0);
    } else {
        tankRightKg_ = safety::SanitizeDouble(tankRightKg_ - safeMass, 0.0, kMaxUsableFuelKg, 0.0);
    }
}

double FuelSystem::tankQuantityKg(FuelTank tank) const noexcept {
    return tank == FuelTank::Left ? tankLeftKg_ : tankRightKg_;
}

double FuelSystem::totalFuelKg() const noexcept {
    return tankLeftKg_ + tankRightKg_;
}

bool FuelSystem::isPumpActive(FuelTank tank, bool primary) const noexcept {
    if (tank == FuelTank::Left) {
        return primary ? pumpLeftPrimaryActive_ : pumpLeftStandbyActive_;
    }
    return primary ? pumpRightPrimaryActive_ : pumpRightStandbyActive_;
}

bool FuelSystem::isCrossfeedOpen() const noexcept {
    return crossfeedValveOpen_;
}

double FuelSystem::imbalanceKg() const noexcept {
    return std::fabs(tankLeftKg_ - tankRightKg_);
}

void FuelSystem::clampTankQuantities() noexcept {
    tankLeftKg_ = safety::SanitizeDouble(tankLeftKg_, 0.0, kMaxUsableFuelKg, 0.0);
    tankRightKg_ = safety::SanitizeDouble(tankRightKg_, 0.0, kMaxUsableFuelKg, 0.0);
}

} // namespace e195e2::systems
