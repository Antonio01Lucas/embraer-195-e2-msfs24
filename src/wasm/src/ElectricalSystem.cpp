#include "ElectricalSystem.h"

// Implementação mínima para tornar a classe linkável/testável nesta fase.
// Lógica de priorização de barramento REAL (qual gerador alimenta qual bus
// em cada combinação de falha) e load shedding por prioridade de carga
// permanecem TODO(Fase 1) — ver ElectricalSystem.h.

namespace e195e2::systems {

void ElectricalSystem::update(double dt) noexcept {
    const double safeDt = safety::SanitizeDeltaTime(dt);
    (void)safeDt; // reservado para resposta transitória de barramento (Fase 1)
    recomputeBusVoltages();
}

void ElectricalSystem::setGeneratorOnline(GeneratorSource source, bool online) noexcept {
    switch (source) {
        case GeneratorSource::Idg1:
            idg1Online_ = online && engine1Running_;
            break;
        case GeneratorSource::Idg2:
            idg2Online_ = online && engine2Running_;
            break;
        case GeneratorSource::ApuGenerator:
            apuGeneratorOnline_ = online;
            break;
        case GeneratorSource::Rat:
            ratDeployed_ = online;
            break;
    }
}

void ElectricalSystem::setEngineRunning(int engineIndex, bool running) noexcept {
    if (engineIndex == 1) {
        engine1Running_ = running;
        if (!running) {
            idg1Online_ = false;
        }
    } else if (engineIndex == 2) {
        engine2Running_ = running;
        if (!running) {
            idg2Online_ = false;
        }
    }
}

double ElectricalSystem::busVoltage(ElectricalBus bus) const noexcept {
    switch (bus) {
        case ElectricalBus::AC1: return busVoltageAc1_;
        case ElectricalBus::AC2: return busVoltageAc2_;
        case ElectricalBus::ACEssential: return busVoltageAcEssential_;
        case ElectricalBus::DC1: return busVoltageDc1_;
        case ElectricalBus::DC2: return busVoltageDc2_;
        case ElectricalBus::DCEssential: return busVoltageDcEssential_;
        case ElectricalBus::DCEmergency: return busVoltageDcEmergency_;
    }
    return 0.0;
}

bool ElectricalSystem::isBusPowered(ElectricalBus bus) const noexcept {
    // Limiar de 1V para tratar ruído numérico como "não energizado".
    return busVoltage(bus) > 1.0;
}

bool ElectricalSystem::isGeneratorOnline(GeneratorSource source) const noexcept {
    switch (source) {
        case GeneratorSource::Idg1: return idg1Online_;
        case GeneratorSource::Idg2: return idg2Online_;
        case GeneratorSource::ApuGenerator: return apuGeneratorOnline_;
        case GeneratorSource::Rat: return ratDeployed_;
    }
    return false;
}

double ElectricalSystem::batteryChargePercent(int batteryIndex) const noexcept {
    if (batteryIndex == 1) return battery1ChargePercent_;
    if (batteryIndex == 2) return battery2ChargePercent_;
    return 0.0;
}

bool ElectricalSystem::isLoadSheddingActive() const noexcept {
    return loadSheddingActive_;
}

void ElectricalSystem::recomputeBusVoltages() noexcept {
    busVoltageAc1_ = idg1Online_ ? kNominalAcVolts : 0.0;
    busVoltageAc2_ = idg2Online_ ? kNominalAcVolts : 0.0;

    const bool acAvailable = idg1Online_ || idg2Online_ || apuGeneratorOnline_;
    busVoltageAcEssential_ = acAvailable
        ? kNominalAcVolts
        : (ratDeployed_ ? kNominalAcVolts * 0.9 : 0.0); // RAT: tensão reduzida (estimativa)

    busVoltageDc1_ = idg1Online_
        ? kNominalDcVolts
        : (battery1ChargePercent_ > 0.0 ? kNominalBatteryVolts : 0.0);
    busVoltageDc2_ = idg2Online_
        ? kNominalDcVolts
        : (battery2ChargePercent_ > 0.0 ? kNominalBatteryVolts : 0.0);
    busVoltageDcEssential_ = (busVoltageDc1_ > 0.0 || busVoltageDc2_ > 0.0) ? kNominalDcVolts : 0.0;
    busVoltageDcEmergency_ =
        (battery1ChargePercent_ > 0.0 || battery2ChargePercent_ > 0.0) ? kNominalBatteryVolts : 0.0;

    loadSheddingActive_ = !acAvailable && !ratDeployed_;

    busVoltageAc1_ = safety::SanitizeDouble(busVoltageAc1_, 0.0, kNominalAcVolts, 0.0);
    busVoltageAc2_ = safety::SanitizeDouble(busVoltageAc2_, 0.0, kNominalAcVolts, 0.0);
    busVoltageAcEssential_ = safety::SanitizeDouble(busVoltageAcEssential_, 0.0, kNominalAcVolts, 0.0);
    busVoltageDc1_ = safety::SanitizeDouble(busVoltageDc1_, 0.0, kNominalDcVolts, 0.0);
    busVoltageDc2_ = safety::SanitizeDouble(busVoltageDc2_, 0.0, kNominalDcVolts, 0.0);
    busVoltageDcEssential_ = safety::SanitizeDouble(busVoltageDcEssential_, 0.0, kNominalDcVolts, 0.0);
    busVoltageDcEmergency_ = safety::SanitizeDouble(busVoltageDcEmergency_, 0.0, kNominalBatteryVolts, 0.0);
}

} // namespace e195e2::systems
