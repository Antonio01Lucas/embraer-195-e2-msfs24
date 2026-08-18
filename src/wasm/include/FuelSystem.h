// Ref: ATA 28 - Fuel System (ver roadmap-sistemas-ata.md, seção 2)
//
// Tanques Left/Right com Collector Boxes, bombas elétricas AC primárias,
// bomba de partida DC, válvulas de crossfeed, balanceamento por
// gravidade/pressão. Capacidade máxima utilizável: 13.690 kg (fonte:
// folheto oficial Embraer — ver dados-tecnicos-extraidos.md).
#pragma once

#include "ISystem.h"
#include "SafetyGuards.h"

namespace e195e2::systems {

enum class FuelTank {
    Left,
    Right,
};

class FuelSystem final : public ISystem {
public:
    FuelSystem() = default;

    void update(double dt) noexcept override;

    void setTankQuantityKg(FuelTank tank, double quantityKg) noexcept;
    void setPumpActive(FuelTank tank, bool primary, bool active) noexcept;
    void setCrossfeedValveOpen(bool open) noexcept;
    void consumeFuel(FuelTank tank, double massKg) noexcept;

    [[nodiscard]] double tankQuantityKg(FuelTank tank) const noexcept;
    [[nodiscard]] double totalFuelKg() const noexcept;
    [[nodiscard]] bool isPumpActive(FuelTank tank, bool primary) const noexcept;
    [[nodiscard]] bool isCrossfeedOpen() const noexcept;
    [[nodiscard]] double imbalanceKg() const noexcept; // |L - R|

private:
    // Ref: dados-tecnicos-extraidos.md — folheto oficial Embraer.
    static constexpr double kMaxUsableFuelKg = 13'690.0;
    static constexpr double kMaxImbalanceWarningKg = 453.0; // ~1000 lb, valor
        // de referência genérico da indústria para jatos regionais — a
        // confirmar contra QRH real do E195-E2.

    double tankLeftKg_ = 0.0;
    double tankRightKg_ = 0.0;

    bool pumpLeftPrimaryActive_ = false;
    bool pumpLeftStandbyActive_ = false;
    bool pumpRightPrimaryActive_ = false;
    bool pumpRightStandbyActive_ = false;

    bool crossfeedValveOpen_ = false;

    // TODO(Fase 1): lógica real de balanceamento por gravidade/pressão via
    // crossfeed, sequenciamento de bombas primária/standby, e alarme de
    // desbalanceamento (CAS message) quando imbalanceKg() > threshold.
    void clampTankQuantities() noexcept;
};

} // namespace e195e2::systems
