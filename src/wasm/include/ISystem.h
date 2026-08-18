// Ref: Regra de Conduta do Projeto #2 - "Modularidade Estrita"
//
// Interface comum a todos os sistemas físicos do core. Força que cada
// sistema exponha um ciclo de atualização determinístico e único ponto de
// entrada por frame — nada de sistemas lendo/escrevendo estado um do outro
// diretamente; a integração entre sistemas (ex: FBW consumindo pressão
// hidráulica) deve passar por uma camada de barramento explícita (a ser
// definida na Fase 1), não por acoplamento direto entre classes.
#pragma once

namespace e195e2::systems {

class ISystem {
public:
    ISystem() = default;
    virtual ~ISystem() = default;

    ISystem(const ISystem&) = delete;
    ISystem& operator=(const ISystem&) = delete;
    ISystem(ISystem&&) = delete;
    ISystem& operator=(ISystem&&) = delete;

    // Atualização cíclica determinística. dt em segundos, já deve chegar
    // sanitizado (ver SafetyGuards::SanitizeDeltaTime) antes de propagar
    // para os sistemas — cada update() ainda re-sanitiza dt internamente
    // por segurança (defesa em profundidade: nenhum sistema deve confiar
    // cegamente que o chamador sanitizou corretamente).
    virtual void update(double dt) noexcept = 0;
};

} // namespace e195e2::systems
