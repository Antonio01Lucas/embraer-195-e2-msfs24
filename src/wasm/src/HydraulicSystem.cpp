#include "HydraulicSystem.h"

// Implementação mínima: resposta transitória de pressão via rate-limit
// (constante de tempo estimada, não confirmada contra AMM real) e PTU
// simplificado (transferência parcial de pressão, não modela vazão real).
// Ver TODO(Fase 1) em HydraulicSystem.h para o modelo completo.

namespace e195e2::systems {

namespace {
// Ref: estimativa genérica de resposta de sistema hidráulico de aviação —
// NÃO confirmada contra AMM/FCOM real do E195-E2.
constexpr double kAccumulatorTimeConstantSeconds = 3.0;
}

void HydraulicSystem::update(double dt) noexcept {
    const double safeDt = safety::SanitizeDeltaTime(dt);
    recomputePressures(safeDt);
}

void HydraulicSystem::setEnginePumpActive(HydraulicSystemId system, bool active) noexcept {
    if (system == HydraulicSystemId::System1) {
        edp1Active_ = active;
    } else if (system == HydraulicSystemId::System2) {
        edp2Active_ = active;
    }
}

void HydraulicSystem::setElectricPumpActive(HydraulicSystemId system, bool active) noexcept {
    switch (system) {
        case HydraulicSystemId::System1: acmp1Active_ = active; break;
        case HydraulicSystemId::System2: acmp2Active_ = active; break;
        case HydraulicSystemId::System3: acmp3aActive_ = active; break;
    }
}

void HydraulicSystem::setPtuActive(bool active) noexcept {
    ptuActive_ = active;
}

void HydraulicSystem::setRatDeployed(bool deployed) noexcept {
    ratDeployed_ = deployed;
}

double HydraulicSystem::pressurePsi(HydraulicSystemId system) const noexcept {
    switch (system) {
        case HydraulicSystemId::System1: return pressureSystem1Psi_;
        case HydraulicSystemId::System2: return pressureSystem2Psi_;
        case HydraulicSystemId::System3: return pressureSystem3Psi_;
    }
    return 0.0;
}

bool HydraulicSystem::isPressurized(HydraulicSystemId system) const noexcept {
    return pressurePsi(system) > kMinPressurizedThresholdPsi;
}

double HydraulicSystem::accumulatorPressurePsi(HydraulicSystemId system) const noexcept {
    switch (system) {
        case HydraulicSystemId::System1: return accumulatorSystem1Psi_;
        case HydraulicSystemId::System2: return accumulatorSystem2Psi_;
        case HydraulicSystemId::System3: return accumulatorSystem3Psi_;
    }
    return 0.0;
}

void HydraulicSystem::recomputePressures(double dt) noexcept {
    const bool sys1PumpActive = edp1Active_ || acmp1Active_;
    const bool sys2PumpActive = edp2Active_ || acmp2Active_;
    const bool sys3PumpActive = acmp3aActive_ || acmp3bActive_;

    double targetSys1 = sys1PumpActive ? kNominalPressurePsi : 0.0;
    double targetSys2 = sys2PumpActive ? kNominalPressurePsi : 0.0;
    const double targetSys3 = sys3PumpActive
        ? kNominalPressurePsi
        : (ratDeployed_ ? kNominalPressurePsi * 0.5 : 0.0);

    // PTU: transferência PARCIAL e simplificada de pressão entre Sistema 1 e
    // 2 quando um lado não tem bomba própria ativa. Não modela vazão real
    // nem o acoplamento mecânico do PTU — TODO(Fase 1).
    if (!sys1PumpActive && sys2PumpActive && ptuActive_) {
        targetSys1 = kNominalPressurePsi * 0.8;
    }
    if (!sys2PumpActive && sys1PumpActive && ptuActive_) {
        targetSys2 = kNominalPressurePsi * 0.8;
    }

    const double maxRatePerSecond = kNominalPressurePsi / kAccumulatorTimeConstantSeconds;

    pressureSystem1Psi_ = safety::RateLimit(pressureSystem1Psi_, targetSys1, maxRatePerSecond, dt);
    pressureSystem2Psi_ = safety::RateLimit(pressureSystem2Psi_, targetSys2, maxRatePerSecond, dt);
    pressureSystem3Psi_ = safety::RateLimit(pressureSystem3Psi_, targetSys3, maxRatePerSecond, dt);

    pressureSystem1Psi_ = safety::SanitizeDouble(pressureSystem1Psi_, 0.0, kMaxTransientPressurePsi, 0.0);
    pressureSystem2Psi_ = safety::SanitizeDouble(pressureSystem2Psi_, 0.0, kMaxTransientPressurePsi, 0.0);
    pressureSystem3Psi_ = safety::SanitizeDouble(pressureSystem3Psi_, 0.0, kMaxTransientPressurePsi, 0.0);

    const double accumulatorRatePerSecond = maxRatePerSecond * 0.5;
    accumulatorSystem1Psi_ = safety::RateLimit(accumulatorSystem1Psi_, pressureSystem1Psi_, accumulatorRatePerSecond, dt);
    accumulatorSystem2Psi_ = safety::RateLimit(accumulatorSystem2Psi_, pressureSystem2Psi_, accumulatorRatePerSecond, dt);
    accumulatorSystem3Psi_ = safety::RateLimit(accumulatorSystem3Psi_, pressureSystem3Psi_, accumulatorRatePerSecond, dt);

    accumulatorSystem1Psi_ = safety::SanitizeDouble(accumulatorSystem1Psi_, 0.0, kMaxTransientPressurePsi, 0.0);
    accumulatorSystem2Psi_ = safety::SanitizeDouble(accumulatorSystem2Psi_, 0.0, kMaxTransientPressurePsi, 0.0);
    accumulatorSystem3Psi_ = safety::SanitizeDouble(accumulatorSystem3Psi_, 0.0, kMaxTransientPressurePsi, 0.0);
}

} // namespace e195e2::systems
