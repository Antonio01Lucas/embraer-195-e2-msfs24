// Ref: MSFS 2024 SDK - docs.flightsimulator.com,
// "6_Programming_APIs/WASM/Vars_API/Vars_API.htm" (LVar functions) e
// "6_Programming_APIs/SimVars/Simulation_Variable_Units.htm" (strings de
// unidade válidas: "percent", "psi", "degrees", "Bool", "number", etc.).
//
// Esta camada NÃO chama a Vars API diretamente - só traduz o estado atual
// do SimBus para a lista de L:Vars a publicar. É PURA (sem dependência
// dos headers da MSFS WASM SDK), por isso é testável com GoogleTest neste
// ambiente, ao contrário do código que realmente chama
// fsVarsRegisterLVar/fsVarsLVarSet (esse fica em tools/msfs/, escrito a
// partir da documentação oficial mas NÃO compilável aqui - requer o SDK
// real do MSFS 2024, não instalado neste ambiente. Validar antes de usar
// em produção.)
//
// Convenção de nome: prefixo "E195E2_" em todo L:Var, para não colidir
// com L:Vars de outros addons (L:Vars são globais no sim inteiro, não têm
// namespace por addon).
//
// Escopo desta primeira leva (elétrico + trem de pouso + flap/slat/
// speedbrake/spoilers via FlyByWire) segue o que foi pedido - hidráulico,
// motor/APU e combustível ficam para uma leva seguinte, mesmo padrão.
#pragma once

#include <array>
#include <cstddef>

#include "SimBus.h"

namespace e195e2::systems {

struct SimVarSample {
    const char* name; // sem o prefixo "L:" - quem publica no sim adiciona
    const char* unit;  // string de unidade da Vars API (ver Simulation_Variable_Units.htm)
    double value;
};

class SimVarPublisher final {
public:
    static constexpr std::size_t kSampleCount = 20;
    using Samples = std::array<SimVarSample, kSampleCount>;

    [[nodiscard]] static Samples collect(const SimBus& bus) noexcept;

private:
    [[nodiscard]] static double gearPositionValue(GearPosition position) noexcept;
    [[nodiscard]] static double controlLawValue(ControlLaw law) noexcept;
};

} // namespace e195e2::systems
