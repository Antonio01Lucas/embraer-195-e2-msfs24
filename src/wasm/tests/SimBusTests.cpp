#include <gtest/gtest.h>

#include "SimBus.h"

using namespace e195e2::systems;

namespace {
void runFor(SimBus& bus, double totalSeconds, double dt = 0.1) {
    for (double t = 0.0; t < totalSeconds; t += dt) {
        bus.update(dt);
    }
}
}

TEST(SimBus, NoSystemsPoweredOrPressurizedAtDefault) {
    SimBus bus;
    bus.update(1.0 / 60.0);

    EXPECT_FALSE(bus.electrical().isBusPowered(ElectricalBus::AC1));
    EXPECT_FALSE(bus.electrical().isBusPowered(ElectricalBus::AC2));
    EXPECT_FALSE(bus.hydraulic().isPressurized(HydraulicSystemId::System1));
    EXPECT_FALSE(bus.hydraulic().isPressurized(HydraulicSystemId::System2));
}

TEST(SimBus, EngineRunningPropagatesToElectricalIdgEligibility) {
    SimBus bus;
    bus.engine(1).setThrottleInput(1.0);
    bus.update(1.0 / 60.0); // engine1.isRunning() já fica true neste update
                             // (ver PowerplantSystem::recomputeEngineState);
                             // o bus já propaga isso para
                             // electrical().setEngineRunning(1, true) antes
                             // de retornar.
    ASSERT_TRUE(bus.engine(1).isRunning());

    // Comando de cockpit separado (o bus NÃO liga o gerador sozinho -
    // ver SimBus.h) - só agora, com engine1Running_ já true no
    // ElectricalSystem, o IDG1 pode ficar online.
    bus.electrical().setGeneratorOnline(GeneratorSource::Idg1, true);
    bus.update(1.0 / 60.0);

    EXPECT_TRUE(bus.electrical().isGeneratorOnline(GeneratorSource::Idg1));
    EXPECT_TRUE(bus.electrical().isBusPowered(ElectricalBus::AC1));
}

TEST(SimBus, StoppingEngineTakesIdgOfflineThroughTheBus) {
    SimBus bus;
    bus.engine(1).setThrottleInput(1.0);
    bus.update(1.0 / 60.0);
    bus.electrical().setGeneratorOnline(GeneratorSource::Idg1, true);
    bus.update(1.0 / 60.0);
    ASSERT_TRUE(bus.electrical().isBusPowered(ElectricalBus::AC1));

    bus.engine(1).setThrottleInput(0.0);
    bus.update(1.0 / 60.0);

    EXPECT_FALSE(bus.engine(1).isRunning());
    EXPECT_FALSE(bus.electrical().isGeneratorOnline(GeneratorSource::Idg1));
    EXPECT_FALSE(bus.electrical().isBusPowered(ElectricalBus::AC1));
}

TEST(SimBus, EngineRunningPressurizesOnlyItsOwnHydraulicSystem) {
    SimBus bus;
    bus.engine(1).setThrottleInput(1.0);
    runFor(bus, 10.0); // constante de tempo do acumulador é 3s (ver
                        // HydraulicSystem.cpp) - 10s é margem suficiente
                        // para convergir.

    EXPECT_TRUE(bus.hydraulic().isPressurized(HydraulicSystemId::System1));
    EXPECT_FALSE(bus.hydraulic().isPressurized(HydraulicSystemId::System2));
}

TEST(SimBus, ApuElectricalAvailabilityPowersEssentialBusWithoutEngines) {
    SimBus bus;
    bus.apu().requestStart();
    bus.apu().setGeneratorOnline(true);
    // kStartSequenceSeconds = 45.0 (ver APUSystem.h) - margem para
    // garantir que o APU chegou a Running.
    runFor(bus, 50.0);

    ASSERT_EQ(bus.apu().state(), ApuState::Running);
    EXPECT_TRUE(bus.electrical().isGeneratorOnline(GeneratorSource::ApuGenerator));
    EXPECT_TRUE(bus.electrical().isBusPowered(ElectricalBus::ACEssential));
    // Nenhum motor rodou nesta sequência - AC1/AC2 (IDG) continuam
    // desenergizados, só o essential via APU.
    EXPECT_FALSE(bus.electrical().isBusPowered(ElectricalBus::AC1));
}

TEST(SimBus, FullStartupSequenceRunsManyFramesWithoutIssues) {
    // Teste de fumaça: liga APU, os dois motores, mexe em trem/FBW, e
    // roda muitos frames em sequência. Não afirma comportamento fino de
    // nenhum sistema (isso já é coberto pelos testes individuais) - o
    // valor aqui é rodar o SimBus inteiro por tempo suficiente para o
    // ASan/UBSan do CI (ver security-audit.yml) pegar qualquer problema
    // de memória/undefined behavior que só apareça com os 8 sistemas
    // interagindo juntos.
    SimBus bus;

    bus.apu().requestStart();
    bus.apu().setGeneratorOnline(true);
    bus.apu().setBleedValveOpen(true);

    bus.engine(1).setThrottleInput(0.8);
    bus.engine(2).setThrottleInput(0.8);

    bus.electrical().setGeneratorOnline(GeneratorSource::Idg1, true);
    bus.electrical().setGeneratorOnline(GeneratorSource::Idg2, true);

    bus.landingGear().setGearLeverDown(false);
    bus.flyByWire().setPitchCommand(0.3);
    bus.flyByWire().setRollCommand(-0.2);

    runFor(bus, 120.0);

    EXPECT_TRUE(bus.engine(1).isRunning());
    EXPECT_TRUE(bus.engine(2).isRunning());
    EXPECT_TRUE(bus.hydraulic().isPressurized(HydraulicSystemId::System1));
    EXPECT_TRUE(bus.hydraulic().isPressurized(HydraulicSystemId::System2));
}
