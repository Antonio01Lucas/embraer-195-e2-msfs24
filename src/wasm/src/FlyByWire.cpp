#include "FlyByWire.h"

#include <algorithm>
#include <cmath>

// Implementação mínima: mapeamento proporcional de comando -> deflexão de
// superfície, com rate-limit para simular resposta de atuador, e proteções
// de envelope como lógica booleana simplificada (não a lei de controle
// real). Ver TODO(Fase 1) em FlyByWire.h para o modelo completo.

namespace e195e2::systems {

namespace {
constexpr double kMaxElevatorDegrees = 20.0;   // estimativa genérica, não confirmada
constexpr double kMaxAileronDegrees = 25.0;    // estimativa genérica, não confirmada
constexpr double kMaxRudderDegrees = 25.0;     // estimativa genérica, não confirmada
constexpr double kSurfaceRatePerSecond = 40.0; // deg/s - estimativa genérica de atuador
constexpr double kFlapRatePerSecondPercent = 5.0;
}

void FlyByWire::update(double dt) noexcept {
    const double safeDt = safety::SanitizeDeltaTime(dt);
    recomputeControlLaw();
    recomputeSurfacePositions(safeDt);
}

void FlyByWire::setPitchCommand(double normalizedInput) noexcept {
    pitchCommand_ = safety::SanitizeDouble(normalizedInput, -1.0, 1.0, 0.0);
}

void FlyByWire::setRollCommand(double normalizedInput) noexcept {
    rollCommand_ = safety::SanitizeDouble(normalizedInput, -1.0, 1.0, 0.0);
}

void FlyByWire::setYawCommand(double normalizedInput) noexcept {
    yawCommand_ = safety::SanitizeDouble(normalizedInput, -1.0, 1.0, 0.0);
}

void FlyByWire::setSpeedbrakeCommand(double normalizedInput) noexcept {
    speedbrakeCommand_ = safety::SanitizeDouble(normalizedInput, 0.0, 1.0, 0.0);
}

void FlyByWire::setFlapLeverPosition(int detentIndex) noexcept {
    flapDetentIndex_ = std::clamp(detentIndex, 0, 5);
}

void FlyByWire::setCurrentAngleOfAttackDegrees(double aoaDegrees) noexcept {
    currentAoaDegrees_ = safety::SanitizeDouble(aoaDegrees, -30.0, 60.0, 0.0);
}

void FlyByWire::setCurrentBankAngleDegrees(double bankDegrees) noexcept {
    currentBankDegrees_ = safety::SanitizeDouble(bankDegrees, -180.0, 180.0, 0.0);
}

void FlyByWire::setCurrentPitchAttitudeDegrees(double pitchDegrees) noexcept {
    currentPitchDegrees_ = safety::SanitizeDouble(pitchDegrees, -90.0, 90.0, 0.0);
}

void FlyByWire::forceControlLaw(ControlLaw law) noexcept {
    activeControlLaw_ = law;
    controlLawForced_ = true;
}

ControlLaw FlyByWire::activeControlLaw() const noexcept {
    return activeControlLaw_;
}

const ControlSurfacePositions& FlyByWire::surfacePositions() const noexcept {
    return surfacePositions_;
}

bool FlyByWire::isHighAoaProtectionActive() const noexcept {
    return highAoaProtectionActive_;
}

bool FlyByWire::isBankLimitProtectionActive() const noexcept {
    return bankLimitProtectionActive_;
}

void FlyByWire::recomputeControlLaw() noexcept {
    // TODO(Fase 1): degradação automática Normal -> Direct sob falha de
    // sensor redundante. Por ora, só forceControlLaw() muda a lei ativa.
    if (!controlLawForced_) {
        activeControlLaw_ = ControlLaw::Normal;
    }

    const bool protectionsEnabled = (activeControlLaw_ == ControlLaw::Normal);
    highAoaProtectionActive_ = protectionsEnabled && (currentAoaDegrees_ >= kHighAoaProtectionThresholdDegrees);
    bankLimitProtectionActive_ = protectionsEnabled && (std::fabs(currentBankDegrees_) >= kMaxBankAngleNormalDegrees);
}

void FlyByWire::recomputeSurfacePositions(double dt) noexcept {
    double elevatorTarget = pitchCommand_ * kMaxElevatorDegrees;
    if (highAoaProtectionActive_) {
        // Simplificação grosseira da proteção de alto AoA: satura o
        // comando de cabrar. A lei real do FCS não é modelada aqui.
        elevatorTarget = std::min(elevatorTarget, -2.0);
    }

    double aileronTarget = rollCommand_ * kMaxAileronDegrees;
    if (bankLimitProtectionActive_) {
        // Amortece o comando de rolagem quando o banco excede o limite —
        // simplificação grosseira, não a lei real de bank protection.
        aileronTarget *= 0.3;
    }

    const double rudderTarget = yawCommand_ * kMaxRudderDegrees;

    surfacePositions_.elevatorDegrees =
        safety::RateLimit(surfacePositions_.elevatorDegrees, elevatorTarget, kSurfaceRatePerSecond, dt);
    surfacePositions_.aileronLeftDegrees =
        safety::RateLimit(surfacePositions_.aileronLeftDegrees, aileronTarget, kSurfaceRatePerSecond, dt);
    surfacePositions_.aileronRightDegrees =
        safety::RateLimit(surfacePositions_.aileronRightDegrees, -aileronTarget, kSurfaceRatePerSecond, dt);
    surfacePositions_.rudderDegrees =
        safety::RateLimit(surfacePositions_.rudderDegrees, rudderTarget, kSurfaceRatePerSecond, dt);

    surfacePositions_.speedbrakePercent =
        safety::SanitizeDouble(speedbrakeCommand_ * 100.0, 0.0, 100.0, 0.0);

    // Mapeamento de detente -> percentual simplificado (0,20,40,60,80,100).
    // TODO(Fase 1): tabela real de detentes de flap/slat do E195-E2.
    const double flapTarget = static_cast<double>(flapDetentIndex_) * 20.0;
    surfacePositions_.flapPercent =
        safety::RateLimit(surfacePositions_.flapPercent, flapTarget, kFlapRatePerSecondPercent, dt);
    // Simplificação: slat acompanha o mesmo alvo do flap — scheduling real
    // é TODO(Fase 1).
    surfacePositions_.slatPercent = surfacePositions_.flapPercent;
}

} // namespace e195e2::systems
