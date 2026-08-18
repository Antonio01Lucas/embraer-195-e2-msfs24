#include <gtest/gtest.h>

#include "ElectricalSystem.h"

using namespace e195e2::systems;

TEST(ElectricalSystem, StartsWithLoadSheddingActiveAndNoAcPower) {
    ElectricalSystem elec;
    elec.update(1.0 / 60.0);
    EXPECT_FALSE(elec.isBusPowered(ElectricalBus::AC1));
    EXPECT_FALSE(elec.isBusPowered(ElectricalBus::AC2));
    EXPECT_TRUE(elec.isLoadSheddingActive());
}

TEST(ElectricalSystem, Idg1OnlineRequiresEngine1Running) {
    ElectricalSystem elec;
    // Gerador comandado online mas motor parado: não deve energizar.
    elec.setGeneratorOnline(GeneratorSource::Idg1, true);
    elec.update(1.0 / 60.0);
    EXPECT_FALSE(elec.isGeneratorOnline(GeneratorSource::Idg1));
    EXPECT_FALSE(elec.isBusPowered(ElectricalBus::AC1));
}

TEST(ElectricalSystem, Idg1PowersAc1AndDc1WhenEngineRunning) {
    ElectricalSystem elec;
    elec.setEngineRunning(1, true);
    elec.setGeneratorOnline(GeneratorSource::Idg1, true);
    elec.update(1.0 / 60.0);

    EXPECT_TRUE(elec.isGeneratorOnline(GeneratorSource::Idg1));
    EXPECT_TRUE(elec.isBusPowered(ElectricalBus::AC1));
    EXPECT_TRUE(elec.isBusPowered(ElectricalBus::DC1));
    EXPECT_TRUE(elec.isBusPowered(ElectricalBus::ACEssential));
    EXPECT_FALSE(elec.isLoadSheddingActive());
}

TEST(ElectricalSystem, StoppingEngineTakesGeneratorOffline) {
    ElectricalSystem elec;
    elec.setEngineRunning(1, true);
    elec.setGeneratorOnline(GeneratorSource::Idg1, true);
    elec.update(1.0 / 60.0);
    ASSERT_TRUE(elec.isBusPowered(ElectricalBus::AC1));

    elec.setEngineRunning(1, false);
    elec.update(1.0 / 60.0);
    EXPECT_FALSE(elec.isGeneratorOnline(GeneratorSource::Idg1));
    EXPECT_FALSE(elec.isBusPowered(ElectricalBus::AC1));
}

TEST(ElectricalSystem, DcEmergencyBusPoweredByBatteriesEvenWithNoGenerators) {
    ElectricalSystem elec;
    elec.update(1.0 / 60.0);
    // Baterias começam com 100% de carga (default) — barramento de
    // emergência deve estar energizado mesmo sem geradores.
    EXPECT_TRUE(elec.isBusPowered(ElectricalBus::DCEmergency));
}
