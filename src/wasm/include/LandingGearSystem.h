// Ref: ATA 32 - Landing Gear, Brakes & Steering (ver roadmap-sistemas-ata.md,
// seção 4)
//
// Recolhimento/extensão normal (hidráulico, depende de HydraulicSystem) e
// extensão de emergência por gravidade. Brake-by-Wire com Anti-Skid e
// Autobrake (RTO, LO, MED, HI). Steer-by-Wire no trem de nariz.
#pragma once

#include "ISystem.h"
#include "SafetyGuards.h"

namespace e195e2::systems {

enum class GearPosition {
    Up,
    Down,
    InTransit,
};

enum class AutobrakeMode {
    Off,
    Rto, // Rejected Takeoff
    Lo,
    Med,
    Hi,
};

class LandingGearSystem final : public ISystem {
public:
    LandingGearSystem() = default;

    void update(double dt) noexcept override;

    void setGearLeverDown(bool down) noexcept;
    void requestEmergencyGravityExtension() noexcept;
    void setAutobrakeMode(AutobrakeMode mode) noexcept;
    void setManualBrakeInput(double leftInput, double rightInput) noexcept; // 0.0-1.0
    void setNoseWheelSteeringInputDegrees(double degrees) noexcept;
    void setHydraulicPressureAvailablePsi(double system1Psi, double system2Psi) noexcept;

    [[nodiscard]] GearPosition nosePosition() const noexcept;
    [[nodiscard]] GearPosition leftMainPosition() const noexcept;
    [[nodiscard]] GearPosition rightMainPosition() const noexcept;
    [[nodiscard]] bool isAntiSkidActive() const noexcept;
    [[nodiscard]] bool isEmergencyExtensionUsed() const noexcept;
    [[nodiscard]] double noseWheelSteeringAngleDegrees() const noexcept;
    [[nodiscard]] AutobrakeMode autobrakeMode() const noexcept;

private:
    // TODO(Fase 1): ângulo máximo real de steer-by-wire do trem de nariz e
    // tempo de ciclo de recolhimento/extensão — placeholders genéricos de
    // jato regional, não confirmados contra AMM/FCOM real.
    static constexpr double kMaxNoseSteeringDegrees = 78.0;
    static constexpr double kGearCycleSeconds = 10.0;

    bool gearLeverDown_ = true;
    bool emergencyExtensionRequested_ = false;
    bool emergencyExtensionUsed_ = false;

    GearPosition noseGear_ = GearPosition::Down;
    GearPosition leftMainGear_ = GearPosition::Down;
    GearPosition rightMainGear_ = GearPosition::Down;

    double hydraulicPressureSystem1Psi_ = 0.0;
    double hydraulicPressureSystem2Psi_ = 0.0;

    AutobrakeMode autobrakeMode_ = AutobrakeMode::Off;
    double manualBrakeLeftInput_ = 0.0;
    double manualBrakeRightInput_ = 0.0;
    bool antiSkidActive_ = true;

    double noseWheelSteeringInputDegrees_ = 0.0;
    double noseWheelSteeringAngleDegrees_ = 0.0;

    // TODO(Fase 1): sequenciamento real de recolhimento (portas, sequência
    // gear-doors-uplock), lógica de anti-skid (modulação de pressão de
    // freio por roda com base em slip ratio), e lógica de autobrake
    // (perfil de desaceleração por modo, não apenas um "nível" fixo).
    void recomputeGearState(double dt) noexcept;
};

} // namespace e195e2::systems
