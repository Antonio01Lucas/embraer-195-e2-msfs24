// Ref: ATA 29 - Hydraulic System (ver roadmap-sistemas-ata.md, seção 2)
//
// 3 sistemas hidráulicos independentes, 3000 PSI nominal (valor de projeto
// informado pelo usuário no roadmap; a confirmar contra AMM/FCOM real —
// ver dados-tecnicos-extraidos.md):
//   Sistema 1: EDP1 (motor esquerdo) + ACMP1 (bomba elétrica AC)
//   Sistema 2: EDP2 (motor direito) + ACMP2 + PTU (Power Transfer Unit)
//   Sistema 3: ACMP3A + ACMP3B (elétricas) + backup via RAT
#pragma once

#include "ISystem.h"
#include "SafetyGuards.h"

namespace e195e2::systems {

enum class HydraulicSystemId {
    System1,
    System2,
    System3,
};

class HydraulicSystem final : public ISystem {
public:
    HydraulicSystem() = default;

    void update(double dt) noexcept override;

    void setEnginePumpActive(HydraulicSystemId system, bool active) noexcept; // EDP1/EDP2
    void setElectricPumpActive(HydraulicSystemId system, bool active) noexcept; // ACMP*
    void setPtuActive(bool active) noexcept;
    void setRatDeployed(bool deployed) noexcept;

    [[nodiscard]] double pressurePsi(HydraulicSystemId system) const noexcept;
    [[nodiscard]] bool isPressurized(HydraulicSystemId system) const noexcept;
    [[nodiscard]] double accumulatorPressurePsi(HydraulicSystemId system) const noexcept;

private:
    // Ref: roadmap-sistemas-ata.md (ATA 29) — pressão nominal de projeto.
    static constexpr double kNominalPressurePsi = 3000.0;
    static constexpr double kMaxTransientPressurePsi = 3200.0;
    static constexpr double kMinPressurizedThresholdPsi = 1500.0;

    bool edp1Active_ = false;
    bool edp2Active_ = false;
    bool acmp1Active_ = false;
    bool acmp2Active_ = false;
    bool acmp3aActive_ = false;
    bool acmp3bActive_ = false;
    bool ptuActive_ = false;
    bool ratDeployed_ = false;

    double pressureSystem1Psi_ = 0.0;
    double pressureSystem2Psi_ = 0.0;
    double pressureSystem3Psi_ = 0.0;

    double accumulatorSystem1Psi_ = 0.0;
    double accumulatorSystem2Psi_ = 0.0;
    double accumulatorSystem3Psi_ = 0.0;

    // TODO(Fase 1): modelar a resposta transitória real de pressão
    // (subida/queda com constante de tempo da bomba), lógica de PTU
    // (transferência de potência entre Sistema 1 e 2 sem transferir
    // fluido), e depleção de acumulador sob demanda de freios/superfícies.
    void recomputePressures(double dt) noexcept;
};

} // namespace e195e2::systems
