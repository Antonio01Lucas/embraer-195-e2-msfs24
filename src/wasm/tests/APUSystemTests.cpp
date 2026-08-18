#include <gtest/gtest.h>

#include "APUSystem.h"

using namespace e195e2::systems;

namespace {
// dt <= 0.25s: SanitizeDeltaTime() (SafetyGuards.h) capa qualquer dt maior
// que 0.25s por design (proteção contra integração explosiva após um frame
// lento) — um dt de passo maior que isso faria o tempo simulado real
// divergir de `totalSeconds` aqui no teste.
void runFor(APUSystem& apu, double totalSeconds, double dt = 0.1) {
    for (double t = 0.0; t < totalSeconds; t += dt) {
        apu.update(dt);
    }
}
}

TEST(APUSystem, StartsOff) {
    APUSystem apu;
    apu.update(1.0 / 60.0);
    EXPECT_EQ(apu.state(), ApuState::Off);
    EXPECT_FALSE(apu.hasFault());
}

TEST(APUSystem, RequestStartTransitionsThroughStartingToRunning) {
    APUSystem apu;
    apu.requestStart();
    apu.update(1.0 / 60.0);
    EXPECT_EQ(apu.state(), ApuState::Starting);

    // Sequência de partida de referência (kStartSequenceSeconds = 45s no
    // header) — roda além disso para garantir convergência.
    runFor(apu, 50.0);
    EXPECT_EQ(apu.state(), ApuState::Running);
    EXPECT_GT(apu.n1Percent(), 95.0);
}

TEST(APUSystem, RequestStopReturnsToOff) {
    APUSystem apu;
    apu.requestStart();
    runFor(apu, 50.0);
    ASSERT_EQ(apu.state(), ApuState::Running);

    apu.requestStop();
    // A rampa de desligamento usa a mesma taxa da partida
    // (kNominalRunningN1Percent / kStartSequenceSeconds), então leva ~45s
    // para sair de ~100% N1 até o corte — não é instantânea nem mais
    // rápida que a partida.
    runFor(apu, 50.0);
    EXPECT_EQ(apu.state(), ApuState::Off);
    EXPECT_NEAR(apu.n1Percent(), 0.0, 1.0);
}

TEST(APUSystem, BleedAndElectricalOnlyAvailableWhenRunningAndEnabled) {
    APUSystem apu;
    apu.setBleedValveOpen(true);
    apu.setGeneratorOnline(true);
    EXPECT_FALSE(apu.isAvailableForBleed());
    EXPECT_FALSE(apu.isAvailableForElectrical());

    apu.requestStart();
    runFor(apu, 50.0);
    ASSERT_EQ(apu.state(), ApuState::Running);
    EXPECT_TRUE(apu.isAvailableForBleed());
    EXPECT_TRUE(apu.isAvailableForElectrical());
}
