#include "APUSystem.h"

// Implementação mínima: máquina de estados de start/stop com rampa linear
// de N1/EGT via rate-limit. Curva real de aceleração (não-linear) e reset
// manual de falha permanecem TODO(Fase 1) — ver APUSystem.h.

namespace e195e2::systems {

namespace {
constexpr double kRunningEgtCelsius = 400.0; // estimativa genérica de EGT estabilizado
constexpr double kOffEgtCelsius = 15.0;
constexpr double kEgtRatePerSecond = 60.0; // °C/s - estimativa genérica
}

void APUSystem::update(double dt) noexcept {
    const double safeDt = safety::SanitizeDeltaTime(dt);
    recomputeApuState(safeDt);
}

void APUSystem::requestStart() noexcept {
    if (state_ == ApuState::Off) {
        state_ = ApuState::Starting;
        startSequenceElapsedSeconds_ = 0.0;
    }
    // A partir de FaultShutdown, requestStart() é ignorado — reset manual
    // do piloto ainda não implementado (TODO Fase 1, ver APUSystem.h).
}

void APUSystem::requestStop() noexcept {
    if (state_ == ApuState::Starting || state_ == ApuState::Running) {
        state_ = ApuState::Stopping;
    }
}

void APUSystem::setBleedValveOpen(bool open) noexcept {
    bleedValveOpen_ = open;
}

void APUSystem::setGeneratorOnline(bool online) noexcept {
    generatorOnline_ = online;
}

ApuState APUSystem::state() const noexcept {
    return state_;
}

double APUSystem::n1Percent() const noexcept {
    return n1Percent_;
}

double APUSystem::egtCelsius() const noexcept {
    return egtCelsius_;
}

bool APUSystem::isAvailableForBleed() const noexcept {
    return state_ == ApuState::Running && bleedValveOpen_;
}

bool APUSystem::isAvailableForElectrical() const noexcept {
    return state_ == ApuState::Running && generatorOnline_;
}

bool APUSystem::hasFault() const noexcept {
    return faultLatched_;
}

void APUSystem::recomputeApuState(double dt) noexcept {
    const double n1RatePerSecond = kNominalRunningN1Percent / kStartSequenceSeconds;

    switch (state_) {
        case ApuState::Off:
            n1Percent_ = safety::RateLimit(n1Percent_, 0.0, n1RatePerSecond, dt);
            egtCelsius_ = safety::RateLimit(egtCelsius_, kOffEgtCelsius, kEgtRatePerSecond, dt);
            break;

        case ApuState::Starting:
            startSequenceElapsedSeconds_ += dt;
            n1Percent_ = safety::RateLimit(n1Percent_, kNominalRunningN1Percent, n1RatePerSecond, dt);
            egtCelsius_ = safety::RateLimit(egtCelsius_, kRunningEgtCelsius, kEgtRatePerSecond, dt);
            if (startSequenceElapsedSeconds_ >= kStartSequenceSeconds) {
                state_ = ApuState::Running;
            }
            break;

        case ApuState::Running:
            n1Percent_ = safety::RateLimit(n1Percent_, kNominalRunningN1Percent, n1RatePerSecond, dt);
            egtCelsius_ = safety::RateLimit(egtCelsius_, kRunningEgtCelsius, kEgtRatePerSecond, dt);
            break;

        case ApuState::Stopping:
            n1Percent_ = safety::RateLimit(n1Percent_, 0.0, n1RatePerSecond, dt);
            egtCelsius_ = safety::RateLimit(egtCelsius_, kOffEgtCelsius, kEgtRatePerSecond, dt);
            if (n1Percent_ < 1.0) {
                state_ = ApuState::Off;
                bleedValveOpen_ = false;
                generatorOnline_ = false;
            }
            break;

        case ApuState::FaultShutdown:
            n1Percent_ = safety::RateLimit(n1Percent_, 0.0, n1RatePerSecond, dt);
            bleedValveOpen_ = false;
            generatorOnline_ = false;
            break;
    }

    n1Percent_ = safety::SanitizeDouble(n1Percent_, 0.0, 120.0, 0.0);
    egtCelsius_ = safety::SanitizeDouble(egtCelsius_, -60.0, 1200.0, kOffEgtCelsius);

    if (n1Percent_ > kOverspeedShutdownN1Percent || egtCelsius_ > kOvertempShutdownEgtCelsius) {
        state_ = ApuState::FaultShutdown;
        faultLatched_ = true;
    }
}

} // namespace e195e2::systems
