// Ref: ATA 24 - Electrical System (ver roadmap-sistemas-ata.md, seção 2)
//
// Geração AC: 2x IDG (Integrated Drive Generator, um por motor PW1900G),
// 1x gerador do APU, RAT (Ram Air Turbine) como fonte de emergência.
// Distribuição DC: TRUs (Transformer Rectifier Units) + Baterias 1 e 2.
// Barramentos: AC1, AC2, AC Essential, DC1, DC2, DC Essential, DC Emergency.
//
// Valores nominais usados abaixo (115V AC / 400Hz, 28V DC, 24V bateria) são
// padrões de indústria para aeronaves de transporte a jato modernas — não
// específicos do E195-E2 confirmados via FCOM/AMM real (pendência registrada
// em dados-tecnicos-extraidos.md). Tratar como ponto de partida de engenharia,
// a validar.
#pragma once

#include "ISystem.h"
#include "SafetyGuards.h"

namespace e195e2::systems {

enum class ElectricalBus {
    AC1,
    AC2,
    ACEssential,
    DC1,
    DC2,
    DCEssential,
    DCEmergency,
};

enum class GeneratorSource {
    Idg1,
    Idg2,
    ApuGenerator,
    Rat,
};

class ElectricalSystem final : public ISystem {
public:
    ElectricalSystem() = default;

    void update(double dt) noexcept override;

    // --- Comandos de entrada (vindos do cockpit / outros sistemas) --------
    void setGeneratorOnline(GeneratorSource source, bool online) noexcept;
    void setEngineRunning(int engineIndex, bool running) noexcept; // 1 ou 2

    // --- Leituras (já sanitizadas) -----------------------------------------
    [[nodiscard]] double busVoltage(ElectricalBus bus) const noexcept;
    [[nodiscard]] bool isBusPowered(ElectricalBus bus) const noexcept;
    [[nodiscard]] bool isGeneratorOnline(GeneratorSource source) const noexcept;
    [[nodiscard]] double batteryChargePercent(int batteryIndex) const noexcept; // 1 ou 2
    [[nodiscard]] bool isLoadSheddingActive() const noexcept;

private:
    // Tensões nominais de referência (padrão de indústria — ver nota acima).
    static constexpr double kNominalAcVolts = 115.0;
    static constexpr double kNominalDcVolts = 28.0;
    static constexpr double kNominalBatteryVolts = 24.0;

    bool idg1Online_ = false;
    bool idg2Online_ = false;
    bool apuGeneratorOnline_ = false;
    bool ratDeployed_ = false;

    bool engine1Running_ = false;
    bool engine2Running_ = false;

    double battery1ChargePercent_ = 100.0;
    double battery2ChargePercent_ = 100.0;

    double busVoltageAc1_ = 0.0;
    double busVoltageAc2_ = 0.0;
    double busVoltageAcEssential_ = 0.0;
    double busVoltageDc1_ = 0.0;
    double busVoltageDc2_ = 0.0;
    double busVoltageDcEssential_ = 0.0;
    double busVoltageDcEmergency_ = 0.0;

    bool loadSheddingActive_ = false;

    // TODO(Fase 1): lógica real de priorização de barramento (qual gerador
    // alimenta qual bus em cada combinação de falha), load shedding
    // automático por prioridade de carga, e transferência de barramento
    // (bus tie) — hoje este método apenas mantém o esqueleto seguro.
    void recomputeBusVoltages() noexcept;
};

} // namespace e195e2::systems
