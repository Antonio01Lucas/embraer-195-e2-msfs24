// NÃO FAZ PARTE DO BUILD VERIFICADO (não incluído em CMakeLists.txt de
// propósito). Requer os headers reais do MSFS 2024 WASM SDK (Vars API),
// que não estão instalados neste ambiente — não pôde ser compilado nem
// testado aqui, ao contrário de todo o resto de src/wasm/. Mesmo padrão
// de honestidade já usado para embraer-e195-e2-msfs24.xml e o XML de LOD
// do modelo: escrito a partir da documentação oficial, com fonte citada,
// mas sem verificação de compilação real - validar contra o SDK
// instalado antes de usar em produção.
//
// Fontes (consultadas ao vivo, não de memória):
//   https://docs.flightsimulator.com/msfs2024/html/6_Programming_APIs/WASM/Vars_API/Vars_API.htm
//   .../Vars_API/fsVarsRegisterLVar.htm
//   .../Vars_API/fsVarsLVarSet.htm
//   .../Vars_API/fsVarsGetUnitId.htm
//   .../SimVars/Simulation_Variable_Units.htm (strings de unidade válidas)
//
// O que NÃO foi possível confirmar na documentação consultada (marcado
// como TODO abaixo, não inventado):
//   - Caminho exato do #include do header da Vars API.
//   - Nomes/assinaturas exatos dos callbacks de ciclo de vida do módulo
//     WASM (init/update) na API atual do MSFS 2024 - "Creating WASM
//     Gauges" cita que eles existem, mas não lista as assinaturas. Este
//     arquivo só expõe registerAll()/publish(), para ser chamado de
//     onde quer que esses callbacks reais estejam.
//
// O que FOI confirmado direto da documentação:
//   using FsLVarId = int;                    // Vars_API.htm
//   FsLVarId fsVarsRegisterLVar(const char* name);
//   FsUnitId fsVarsGetUnitId(const char* unitName);
//   FsVarError fsVarsLVarSet(FsLVarId lVarId, FsUnitId unitId, double value);
//   // sucesso: fsVarsLVarSet(...) == FS_VAR_ERROR_NONE (do exemplo oficial)
#pragma once

// TODO: confirmar caminho real, ex. <MSFS/Vars/Vars.h> - não encontrado
// na documentação consultada nesta sessão.
// #include <MSFS/Vars/Vars.h>

#include <array>

#include "SimVarPublisher.h"

namespace e195e2::msfs {

class LVarBridge {
public:
    // Chamar uma vez, no callback de inicialização do módulo WASM (nome
    // exato a confirmar contra o SDK - ver TODO acima). Registra o
    // FsLVarId e o FsUnitId de cada amostra de SimVarPublisher, na mesma
    // ordem, para não precisar procurar por nome a cada frame.
    void registerAll() noexcept;

    // Chamar a cada frame (callback de update do módulo WASM - nome
    // exato a confirmar) com o resultado atual de
    // SimVarPublisher::collect(bus). Publica cada valor via
    // fsVarsLVarSet - o lado HTML/Coherent GT (src/avionics) lê com
    // SimVar.GetSimVarValue("L:<nome>", "number").
    void publish(const e195e2::systems::SimVarPublisher::Samples& samples) noexcept;

private:
    static constexpr std::size_t kCount = e195e2::systems::SimVarPublisher::kSampleCount;

    // FsLVarId/FsUnitId são, pela documentação, typedef de int - usamos
    // int diretamente aqui já que o header real (com os typedefs
    // nomeados) não está disponível para #include neste ambiente.
    std::array<int, kCount> lVarIds_{};
    std::array<int, kCount> unitIds_{};
    bool registered_ = false;
};

} // namespace e195e2::msfs
