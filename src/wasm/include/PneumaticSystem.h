// Ref: ATA 36 - Pneumatic System (ver roadmap-sistemas-ata.md, seção 2)
//
// Sangria dos motores (PW1900G) e do APU, conexão de solo (HP Cart),
// válvulas de crossbleed, isolamento e distribuição pneumática para o
// ECS (ATA 21) e anti-ice (ATA 30).
#pragma once

#include "ISystem.h"
#include "SafetyGuards.h"

namespace e195e2::systems {

class PneumaticSystem final : public ISystem {
public:
    PneumaticSystem() = default;

    void update(double dt) noexcept override;

    void setEngineBleedValveOpen(int engineIndex, bool open) noexcept; // 1 ou 2
    void setApuBleedValveOpen(bool open) noexcept;
    void setCrossbleedValveOpen(bool open) noexcept;
    void setGroundCartConnected(bool connected) noexcept;
    void setIsolationValveOpen(bool open) noexcept;

    [[nodiscard]] double ductPressurePsi(int side) const noexcept; // 1 ou 2 (L/R)
    [[nodiscard]] double ductTemperatureCelsius(int side) const noexcept;
    [[nodiscard]] bool isCrossbleedOpen() const noexcept;
    [[nodiscard]] bool isGroundCartSupplying() const noexcept;

private:
    // TODO(Fase 1): faixas nominais reais de pressão/temperatura de duto —
    // valores abaixo são placeholders de engenharia genéricos (turbofan
    // regional/narrowbody), não confirmados contra AMM/FCOM real.
    static constexpr double kNominalDuctPressurePsi = 30.0;
    static constexpr double kMaxDuctPressurePsi = 45.0;
    static constexpr double kNominalDuctTempCelsius = 200.0;
    static constexpr double kMaxDuctTempCelsius = 260.0;

    bool engine1BleedOpen_ = false;
    bool engine2BleedOpen_ = false;
    bool apuBleedOpen_ = false;
    bool crossbleedValveOpen_ = false;
    bool groundCartConnected_ = false;
    bool isolationValveOpen_ = true;

    double ductPressureLeftPsi_ = 0.0;
    double ductPressureRightPsi_ = 0.0;
    double ductTempLeftCelsius_ = 15.0;
    double ductTempRightCelsius_ = 15.0;

    // TODO(Fase 1): modelar acoplamento com PowerplantSystem (sangria
    // depende de N2/regime do motor) e com APUSystem (sangria do APU
    // indisponível acima de determinada altitude — dado a confirmar).
    void recomputeDuctState(double dt) noexcept;
};

} // namespace e195e2::systems
