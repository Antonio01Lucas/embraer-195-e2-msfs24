// Ref: ISystem.h - "a integração entre sistemas... deve passar por uma
// camada de barramento explícita (a ser definida na Fase 1), não por
// acoplamento direto entre classes." Este é essa camada.
//
// SimBus possui todos os 8 sistemas e é o único lugar do core que conhece
// as dependências cruzadas entre eles. Cada sistema continua sem saber da
// existência dos outros - o SimBus só liga getters de um sistema em
// setters de outro, ambos já existentes na interface pública de cada
// classe (nenhum sistema foi alterado para "abrir" um acesso especial
// para o bus).
//
// Acoplamentos que o SimBus FAZ hoje (ver update() no .cpp para a ordem
// exata e o porquê de cada um):
//   - PowerplantSystem(1/2).isRunning() -> ElectricalSystem.setEngineRunning
//     (IDG só pode ficar online com o motor girando)
//   - APUSystem.isAvailableForElectrical() -> ElectricalSystem.setGeneratorOnline(ApuGenerator, ...)
//   - PowerplantSystem(1/2).isRunning() -> HydraulicSystem.setEnginePumpActive
//     (EDP1/EDP2 são bombas acopladas mecanicamente ao motor correspondente)
//   - HydraulicSystem.pressurePsi(System1/2) -> LandingGearSystem.setHydraulicPressureAvailablePsi
//
// Acoplamentos que o SimBus AINDA NÃO FAZ - não por esquecimento, mas
// porque a interface pública do sistema de origem ou destino ainda não
// expõe o dado necessário (lacuna de API documentada no TODO da própria
// classe, não "inventável" pelo bus):
//   - PowerplantSystem -> FuelSystem (consumo real de combustível):
//     PowerplantSystem ainda não expõe uma taxa de consumo real (só
//     recebe setFuelFlowAvailableKgPerHour como entrada) - ver TODO em
//     PowerplantSystem.h ("modelo termodinâmico real do GTF").
//   - PowerplantSystem/APUSystem -> PneumaticSystem (sangria depende de
//     N2/regime do motor): PneumaticSystem ainda não tem um setter para
//     estado do motor - ver TODO em PneumaticSystem.h.
//   - Qualquer sensor de atitude (AoA/bank/pitch) -> FlyByWire: não existe
//     ainda um sistema de dinâmica de voo que produza esses valores; FBW
//     é atualizado pelo SimBus a cada frame, mas continua recebendo essas
//     entradas de fora (cockpit/físico) até essa peça existir.
//
// Efeito ainda não observável: HydraulicSystem -> LandingGearSystem já
// está fiado acima, mas LandingGearSystem::recomputeGearState() ainda não
// USA a pressão recebida (transição de trem é "instantânea" hoje, ver
// TODO em LandingGearSystem.cpp) - o dado chega correto, só não tem efeito
// observável até aquele TODO ser resolvido.
#pragma once

#include "APUSystem.h"
#include "ElectricalSystem.h"
#include "FlyByWire.h"
#include "FuelSystem.h"
#include "HydraulicSystem.h"
#include "LandingGearSystem.h"
#include "PneumaticSystem.h"
#include "PowerplantSystem.h"

namespace e195e2::systems {

class SimBus final {
public:
    SimBus() noexcept;

    SimBus(const SimBus&) = delete;
    SimBus& operator=(const SimBus&) = delete;
    SimBus(SimBus&&) = delete;
    SimBus& operator=(SimBus&&) = delete;

    // Atualiza todos os 8 sistemas em ordem de dependência e propaga o
    // acoplamento entre eles. Ver .cpp para a ordem exata e a
    // justificativa de cada passo.
    void update(double dt) noexcept;

    [[nodiscard]] PowerplantSystem& engine(int engineIndex) noexcept;             // 1 ou 2
    [[nodiscard]] const PowerplantSystem& engine(int engineIndex) const noexcept; // 1 ou 2

    [[nodiscard]] APUSystem& apu() noexcept { return apu_; }
    [[nodiscard]] const APUSystem& apu() const noexcept { return apu_; }

    [[nodiscard]] ElectricalSystem& electrical() noexcept { return electrical_; }
    [[nodiscard]] const ElectricalSystem& electrical() const noexcept { return electrical_; }

    [[nodiscard]] HydraulicSystem& hydraulic() noexcept { return hydraulic_; }
    [[nodiscard]] const HydraulicSystem& hydraulic() const noexcept { return hydraulic_; }

    [[nodiscard]] PneumaticSystem& pneumatic() noexcept { return pneumatic_; }
    [[nodiscard]] const PneumaticSystem& pneumatic() const noexcept { return pneumatic_; }

    [[nodiscard]] FuelSystem& fuel() noexcept { return fuel_; }
    [[nodiscard]] const FuelSystem& fuel() const noexcept { return fuel_; }

    [[nodiscard]] LandingGearSystem& landingGear() noexcept { return landingGear_; }
    [[nodiscard]] const LandingGearSystem& landingGear() const noexcept { return landingGear_; }

    [[nodiscard]] FlyByWire& flyByWire() noexcept { return flyByWire_; }
    [[nodiscard]] const FlyByWire& flyByWire() const noexcept { return flyByWire_; }

private:
    PowerplantSystem engine1_;
    PowerplantSystem engine2_;
    APUSystem apu_;
    ElectricalSystem electrical_;
    HydraulicSystem hydraulic_;
    PneumaticSystem pneumatic_;
    FuelSystem fuel_;
    LandingGearSystem landingGear_;
    FlyByWire flyByWire_;
};

} // namespace e195e2::systems
