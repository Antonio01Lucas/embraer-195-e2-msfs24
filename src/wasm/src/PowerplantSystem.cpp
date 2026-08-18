#include "PowerplantSystem.h"

// Implementação mínima: N1/N2/EGT convergem para alvo proporcional ao
// throttle via rate-limit (spool-up/down simplificado). Curva termodinâmica
// real do GTF (relação N1/N2, resposta ao Mach/altitude/ISA, Flex/Derate
// reduzindo N1 alvo) é TODO(Fase 1) — ver PowerplantSystem.h.
//
// Nota de arquitetura: para este motor de 2 eixos (fan via caixa de
// engrenagens + spool de alta), Ng (gas generator speed) é aproximado aqui
// como igual a N2 (spool de alta) — simplificação documentada, não uma
// arquitetura de 3 eixos.

namespace e195e2::systems {

namespace {
constexpr double kN1RatePerSecond = 15.0;  // %/s - estimativa genérica de spool-up
constexpr double kN2RatePerSecond = 20.0;  // %/s - estimativa genérica (spool de alta é mais rápido)
constexpr double kEgtRatePerSecond = 150.0; // °C/s - estimativa genérica
constexpr double kIdleEgtCelsius = 200.0;
constexpr double kMaxEgtCelsius = 850.0;
}

PowerplantSystem::PowerplantSystem(int engineIndex) noexcept : engineIndex_(engineIndex) {}

void PowerplantSystem::update(double dt) noexcept {
    const double safeDt = safety::SanitizeDeltaTime(dt);
    recomputeEngineState(safeDt);
}

void PowerplantSystem::setThrottleInput(double normalizedInput) noexcept {
    throttleInput_ = safety::SanitizeDouble(normalizedInput, 0.0, 1.0, 0.0);
}

void PowerplantSystem::setThrustMode(ThrustMode mode) noexcept {
    thrustMode_ = mode;
}

void PowerplantSystem::setDerateTemperatureCelsius(double flexTempCelsius) noexcept {
    flexTempCelsius_ = safety::SanitizeDouble(flexTempCelsius, -60.0, 70.0, 0.0);
}

void PowerplantSystem::setActiveFadecChannel(FadecChannel channel) noexcept {
    activeFadecChannel_ = channel;
}

void PowerplantSystem::setThrustReverserDeployed(bool deployed) noexcept {
    // Trava mecânica só é liberada por lógica de sequência válida — ainda
    // não implementada (TODO Fase 1). Por ora, deployed só é aceito se a
    // trava já estiver destravada por outro caminho (nenhum atualmente),
    // então este setter registra a intenção mas não força o destravamento.
    reverserDeployed_ = deployed && !reverserMechanicallyLocked_;
}

void PowerplantSystem::setFuelFlowAvailableKgPerHour(double fuelFlowKgH) noexcept {
    fuelFlowAvailableKgH_ = safety::SanitizeDouble(fuelFlowKgH, 0.0, 10'000.0, 0.0);
}

double PowerplantSystem::n1Percent() const noexcept { return n1Percent_; }
double PowerplantSystem::n2Percent() const noexcept { return n2Percent_; }
double PowerplantSystem::ngPercent() const noexcept { return ngPercent_; }
double PowerplantSystem::egtCelsius() const noexcept { return egtCelsius_; }
double PowerplantSystem::netThrustNewtons() const noexcept { return netThrustNewtons_; }
bool PowerplantSystem::isThrustReverserLocked() const noexcept { return reverserMechanicallyLocked_; }
bool PowerplantSystem::isRunning() const noexcept { return running_; }

void PowerplantSystem::recomputeEngineState(double dt) noexcept {
    // Simplificação: motor é considerado "comandado a rodar" quando há
    // throttle. TODO(Fase 1): sequência real de partida (starter, light-off,
    // idle stabilization) independente do throttle.
    running_ = throttleInput_ > 0.0;

    const double targetN1 = throttleInput_ * 100.0;
    const double targetN2 = throttleInput_ * 100.0;
    const double targetEgt = running_
        ? kIdleEgtCelsius + throttleInput_ * (kMaxEgtCelsius - kIdleEgtCelsius)
        : 15.0;

    n1Percent_ = safety::RateLimit(n1Percent_, targetN1, kN1RatePerSecond, dt);
    n2Percent_ = safety::RateLimit(n2Percent_, targetN2, kN2RatePerSecond, dt);
    ngPercent_ = n2Percent_; // aproximação documentada (ver comentário no topo do arquivo)
    egtCelsius_ = safety::RateLimit(egtCelsius_, targetEgt, kEgtRatePerSecond, dt);

    n1Percent_ = safety::SanitizeDouble(n1Percent_, 0.0, 110.0, 0.0);
    n2Percent_ = safety::SanitizeDouble(n2Percent_, 0.0, 110.0, 0.0);
    ngPercent_ = safety::SanitizeDouble(ngPercent_, 0.0, 110.0, 0.0);
    egtCelsius_ = safety::SanitizeDouble(egtCelsius_, -60.0, 1100.0, 15.0);

    // Mapeamento linear N1 -> empuxo líquido — placeholder grosseiro.
    // TODO(Fase 1): curva real de empuxo vs. N1/altitude/Mach/ISA.
    netThrustNewtons_ = safety::SanitizeDouble(
        (n1Percent_ / 100.0) * kMaxNetThrustNewtons,
        0.0,
        kMaxNetThrustNewtons,
        0.0
    );
}

} // namespace e195e2::systems
