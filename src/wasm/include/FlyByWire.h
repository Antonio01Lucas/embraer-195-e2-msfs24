// Ref: ATA 27 - Flight Controls (Full Fly-By-Wire) — ver
// roadmap-sistemas-ata.md, seção 1.
//
// E195-E2 possui FBW pleno nos 3 eixos (diferente da 1ª geração E-Jet, que
// tinha ailerons por cabo). Leis de controle: Normal Mode (proteções de
// atitude, compensação de empuxo, proteção de alto AoA e bank limit) com
// degradação para Direct Mode. Superfícies: Elevators, Ailerons, Rudder,
// Spoilers multifuncionais (Ground/Roll/Speedbrake), High Lift System
// (Slats/Flaps). Trim digital: Pitch e Yaw via FCS.
#pragma once

#include "ISystem.h"
#include "SafetyGuards.h"

namespace e195e2::systems {

enum class ControlLaw {
    Normal, // proteções completas de envelope
    Direct, // comando proporcional direto, sem proteção de envelope
};

struct ControlSurfacePositions {
    double elevatorDegrees = 0.0;       // + = nose-up
    double aileronLeftDegrees = 0.0;
    double aileronRightDegrees = 0.0;
    double rudderDegrees = 0.0;
    double groundSpoilerPercent = 0.0;  // 0-100
    double rollSpoilerPercent = 0.0;    // 0-100 (diferencial, por lado)
    double speedbrakePercent = 0.0;     // 0-100
    double slatPercent = 0.0;           // 0-100 (retraído a estendido)
    double flapPercent = 0.0;           // 0-100
    double pitchTrimPercent = -100.0;   // -100 a 100
    double yawTrimPercent = 0.0;        // -100 a 100
};

class FlyByWire final : public ISystem {
public:
    FlyByWire() = default;

    void update(double dt) noexcept override;

    // --- Entradas de comando (side-stick/pedais, já normalizados -1..1) ---
    void setPitchCommand(double normalizedInput) noexcept;
    void setRollCommand(double normalizedInput) noexcept;
    void setYawCommand(double normalizedInput) noexcept;
    void setSpeedbrakeCommand(double normalizedInput) noexcept; // 0..1
    void setFlapLeverPosition(int detentIndex) noexcept;

    // --- Entradas de sensores (para as leis de proteção) -------------------
    void setCurrentAngleOfAttackDegrees(double aoaDegrees) noexcept;
    void setCurrentBankAngleDegrees(double bankDegrees) noexcept;
    void setCurrentPitchAttitudeDegrees(double pitchDegrees) noexcept;

    // --- Degradação de leis --------------------------------------------
    void forceControlLaw(ControlLaw law) noexcept; // uso: falha simulada/QRH

    [[nodiscard]] ControlLaw activeControlLaw() const noexcept;
    [[nodiscard]] const ControlSurfacePositions& surfacePositions() const noexcept;
    [[nodiscard]] bool isHighAoaProtectionActive() const noexcept;
    [[nodiscard]] bool isBankLimitProtectionActive() const noexcept;

private:
    // Ref: roadmap-sistemas-ata.md (ATA 27) — limites de proteção citados
    // pelo usuário (alto AoA e bank limit). Valores numéricos abaixo são
    // placeholders de engenharia (faixa típica de transporte a jato) e
    // NÃO são os limites certificados reais do E195-E2 — a confirmar
    // contra AFM/FCOM real (ver dados-tecnicos-extraidos.md, lacuna #4).
    static constexpr double kMaxBankAngleNormalDegrees = 67.0;
    static constexpr double kHighAoaProtectionThresholdDegrees = 14.0;

    ControlLaw activeControlLaw_ = ControlLaw::Normal;
    bool controlLawForced_ = false;

    double pitchCommand_ = 0.0;
    double rollCommand_ = 0.0;
    double yawCommand_ = 0.0;
    double speedbrakeCommand_ = 0.0;
    int flapDetentIndex_ = 0;

    double currentAoaDegrees_ = 0.0;
    double currentBankDegrees_ = 0.0;
    double currentPitchDegrees_ = 0.0;

    bool highAoaProtectionActive_ = false;
    bool bankLimitProtectionActive_ = false;

    ControlSurfacePositions surfacePositions_{};

    // TODO(Fase 1): implementação real das leis de controle — mapeamento
    // de comando normalizado para deflexão de superfície (ganhos
    // programados por regime de voo), proteção de atitude/AoA/bank como
    // função contínua (não apenas um booleano liga/desliga), e lógica de
    // reversão automática Normal -> Direct sob falha de sensor redundante.
    void recomputeControlLaw() noexcept;
    void recomputeSurfacePositions(double dt) noexcept;
};

} // namespace e195e2::systems
