// Ref: ATA 49 - Auxiliary Power Unit (ver roadmap-sistemas-ata.md, seção 3)
//
// FADEC do APU, sequência de partida, corte automático de segurança por
// sobretemperatura/sobrevelocidade, alimentação de ar (pneumático, ATA 36)
// e eletricidade (ATA 24).
#pragma once

#include "ISystem.h"
#include "SafetyGuards.h"

namespace e195e2::systems {

enum class ApuState {
    Off,
    Starting,
    Running,
    Stopping,
    FaultShutdown,
};

class APUSystem final : public ISystem {
public:
    APUSystem() = default;

    void update(double dt) noexcept override;

    void requestStart() noexcept;
    void requestStop() noexcept;
    void setBleedValveOpen(bool open) noexcept;
    void setGeneratorOnline(bool online) noexcept;

    [[nodiscard]] ApuState state() const noexcept;
    [[nodiscard]] double n1Percent() const noexcept;
    [[nodiscard]] double egtCelsius() const noexcept;
    [[nodiscard]] bool isAvailableForBleed() const noexcept;
    [[nodiscard]] bool isAvailableForElectrical() const noexcept;
    [[nodiscard]] bool hasFault() const noexcept;

private:
    // TODO(Fase 1): limites reais de N1/EGT de corte automático — valores
    // abaixo são placeholders de engenharia genéricos para APU de jato
    // regional, não confirmados contra AMM/FCOM real do E195-E2.
    static constexpr double kOverspeedShutdownN1Percent = 107.0;
    static constexpr double kOvertempShutdownEgtCelsius = 720.0;
    static constexpr double kNominalRunningN1Percent = 100.0;
    static constexpr double kStartSequenceSeconds = 45.0;

    ApuState state_ = ApuState::Off;
    double n1Percent_ = 0.0;
    double egtCelsius_ = 15.0;
    double startSequenceElapsedSeconds_ = 0.0;

    bool bleedValveOpen_ = false;
    bool generatorOnline_ = false;
    bool faultLatched_ = false;

    // TODO(Fase 1): curva real de aceleração N1 durante o start (não
    // linear), lógica de auto-shutdown latch (precisa reset manual do
    // piloto, não só sair da condição de falha), e interação com
    // PneumaticSystem/ElectricalSystem.
    void recomputeApuState(double dt) noexcept;
};

} // namespace e195e2::systems
