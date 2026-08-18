#include "SimBus.h"

namespace e195e2::systems {

SimBus::SimBus() noexcept
    : engine1_(1)
    , engine2_(2)
{
}

PowerplantSystem& SimBus::engine(int engineIndex) noexcept {
    return engineIndex == 1 ? engine1_ : engine2_;
}

const PowerplantSystem& SimBus::engine(int engineIndex) const noexcept {
    return engineIndex == 1 ? engine1_ : engine2_;
}

void SimBus::update(double dt) noexcept {
    // 1) Sistemas "fonte": motores e APU não dependem de nenhum outro
    //    sistema do core hoje (throttle/comandos de partida vêm de fora).
    //    Atualizam primeiro para que o resto do frame já veja o estado
    //    novo (running/N1/etc.) no MESMO frame, sem atraso de 1 frame nos
    //    acoplamentos abaixo.
    engine1_.update(dt);
    engine2_.update(dt);
    apu_.update(dt);

    // 2) Elétrico depende de motor rodando (IDG1/IDG2) e da disponibilidade
    //    elétrica do APU. Note que isto NÃO substitui o comando de cockpit
    //    "gerador online" de cada IDG (ElectricalSystem::setGeneratorOnline)
    //    - só garante que o motor precisa estar girando para o IDG poder
    //    ficar online, que é exatamente o que ElectricalSystem já impõe
    //    internamente (ver Idg1OnlineRequiresEngine1Running).
    electrical_.setEngineRunning(1, engine1_.isRunning());
    electrical_.setEngineRunning(2, engine2_.isRunning());
    electrical_.setGeneratorOnline(GeneratorSource::ApuGenerator, apu_.isAvailableForElectrical());
    electrical_.update(dt);

    // 3) Hidráulico depende de motor rodando (EDP1/EDP2 são bombas
    //    acopladas mecanicamente ao motor correspondente - sem switch de
    //    cockpit separado, diferente do IDG).
    hydraulic_.setEnginePumpActive(HydraulicSystemId::System1, engine1_.isRunning());
    hydraulic_.setEnginePumpActive(HydraulicSystemId::System2, engine2_.isRunning());
    hydraulic_.update(dt);

    // 4) Trem de pouso recebe a pressão hidráulica real disponível (ainda
    //    sem efeito observável até LandingGearSystem consumir esse dado -
    //    ver nota no SimBus.h).
    landingGear_.setHydraulicPressureAvailablePsi(
        hydraulic_.pressurePsi(HydraulicSystemId::System1),
        hydraulic_.pressurePsi(HydraulicSystemId::System2));
    landingGear_.update(dt);

    // 5) Sistemas sem acoplamento "wireable" ainda (lacuna de API do
    //    sistema de origem ou destino, ver SimBus.h) - continuam
    //    recebendo comandos só de fora (cockpit) por enquanto.
    fuel_.update(dt);
    pneumatic_.update(dt);
    flyByWire_.update(dt);
}

} // namespace e195e2::systems
