// Ver aviso de honestidade técnica em lvar_bridge.h - não compilado nem
// testado neste ambiente (sem os headers reais do MSFS 2024 WASM SDK).

#include "lvar_bridge.h"

namespace e195e2::msfs {

void LVarBridge::registerAll() noexcept {
    if (registered_) {
        return;
    }

    // Um SimBus descartável só para ler name/unit de cada amostra (o
    // valor em si é ignorado aqui - registro não depende do estado atual
    // dos sistemas). SimBus é leve (sem alocação em heap), então isso não
    // é desperdício relevante.
    const auto samples = e195e2::systems::SimVarPublisher::collect(e195e2::systems::SimBus{});
    for (std::size_t i = 0; i < kCount; ++i) {
        lVarIds_[i] = fsVarsRegisterLVar(samples[i].name);
        unitIds_[i] = fsVarsGetUnitId(samples[i].unit);
    }

    registered_ = true;
}

void LVarBridge::publish(const e195e2::systems::SimVarPublisher::Samples& samples) noexcept {
    if (!registered_) {
        registerAll();
    }

    for (std::size_t i = 0; i < kCount; ++i) {
        fsVarsLVarSet(lVarIds_[i], unitIds_[i], samples[i].value);
        // Erro de fsVarsLVarSet (!= FS_VAR_ERROR_NONE) intencionalmente
        // ignorado aqui: não há para onde reportar uma falha de
        // publicação de L:Var dentro do loop de update do módulo WASM
        // além de log - TODO quando a integração real com o SDK
        // acontecer (ver lvar_bridge.h).
    }
}

} // namespace e195e2::msfs
