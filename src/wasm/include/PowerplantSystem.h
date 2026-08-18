// Ref: ATA 73/76/78 - Powerplant / FADEC / Thrust Reversers
// Motor: Pratt & Whitney PW1900G (Geared Turbofan) — ver
// dados-tecnicos-extraidos.md para specs cross-checadas (empuxo ~102-104,5 kN
// / ~23.000-23.500 lbf na variante de maior empuxo, bypass ratio ~12:1,
// OPR ~38:1).
//
// FADEC duplo canal (Channel A / Channel B — redundância ativa/standby,
// arquitetura padrão de FADEC dual-channel na indústria).
#pragma once

#include "ISystem.h"
#include "SafetyGuards.h"

namespace e195e2::systems {

enum class ThrustMode {
    Idle,
    Climb,      // CLB
    Cruise,     // CRZ
    Continuous, // CON
    TakeOff1,   // TO-1
    TakeOff2,   // TO-2
};

enum class FadecChannel {
    A,
    B,
};

class PowerplantSystem final : public ISystem {
public:
    explicit PowerplantSystem(int engineIndex) noexcept; // 1 ou 2

    void update(double dt) noexcept override;

    void setThrottleInput(double normalizedInput) noexcept; // 0.0 a 1.0
    void setThrustMode(ThrustMode mode) noexcept;
    void setDerateTemperatureCelsius(double flexTempCelsius) noexcept; // Flex/Assumed Temp
    void setActiveFadecChannel(FadecChannel channel) noexcept;
    void setThrustReverserDeployed(bool deployed) noexcept;
    void setFuelFlowAvailableKgPerHour(double fuelFlowKgH) noexcept; // do FuelSystem

    [[nodiscard]] double n1Percent() const noexcept;
    [[nodiscard]] double n2Percent() const noexcept;
    [[nodiscard]] double ngPercent() const noexcept; // core speed do GTF (gas generator)
    [[nodiscard]] double egtCelsius() const noexcept;
    [[nodiscard]] double netThrustNewtons() const noexcept;
    [[nodiscard]] bool isThrustReverserLocked() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept;

private:
    // Ref: dados-tecnicos-extraidos.md (PW1900G, variante de maior empuxo).
    static constexpr double kMaxNetThrustNewtons = 104'500.0; // ~23.500 lbf

    int engineIndex_;

    double throttleInput_ = 0.0;
    ThrustMode thrustMode_ = ThrustMode::Idle;
    double flexTempCelsius_ = 0.0;
    FadecChannel activeFadecChannel_ = FadecChannel::A;

    bool running_ = false;
    double n1Percent_ = 0.0;
    double n2Percent_ = 0.0;
    double ngPercent_ = 0.0;
    double egtCelsius_ = 15.0;
    double netThrustNewtons_ = 0.0;

    bool reverserDeployed_ = false;
    bool reverserMechanicallyLocked_ = true; // trava de segurança — só destrava
                                              // com sequência válida (Fase 1)

    double fuelFlowAvailableKgH_ = 0.0;

    // TODO(Fase 1): modelo termodinâmico real do GTF (relação N1/N2/Ng,
    // resposta do gerador de gás, curva de empuxo vs. altitude/Mach/ISA,
    // lógica de Flex/Derate reduzindo N1 alvo em vez de "cortar" empuxo
    // linearmente) — hoje apenas esqueleto seguro com saturação via
    // SafetyGuards.
    void recomputeEngineState(double dt) noexcept;
};

} // namespace e195e2::systems
