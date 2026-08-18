// Ref: Regra de Conduta do Projeto #1 - "Física em Primeiro Lugar"
//
// Este cabeçalho concentra as funções de blindagem usadas por TODOS os
// sistemas do core (Elétrico, Hidráulico, Combustível, FBW, FADEC, etc.)
// antes de qualquer valor calculado ser exposto ao SimConnect/LVars ou
// usado como entrada de outro sistema.
//
// Motivação: o core roda em WASM dentro do flight loop do MSFS. Um NaN,
// +/-Inf ou valor fora do domínio físico (ex: pressão hidráulica negativa,
// N1 > 100% por erro de sensor simulado) propagado para os displays ou
// para a lei de controle FBW pode travar a aeronave (efeito visual) ou,
// pior, alimentar uma proteção de envelope com dado inválido. Toda leitura
// de sensor simulado e toda saída de sistema física deve passar por aqui.
#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace e195e2::safety {

// Sanitiza um valor double contra NaN, +/-Inf e fora de faixa físicamente
// válida, retornando um fallback seguro nesses casos.
//
// Uso típico:
//   // Ref: ATA 29 (Hydraulic) - roadmap-sistemas-ata.md - faixa nominal do
//   // Sistema 1: 0 a 3000 PSI (+ margem transitória de pico). NOTA: 3000 PSI
//   // é o valor de projeto informado no roadmap do usuário; ainda não
//   // confirmado contra AMM/FCOM real do E195-E2 (ver
//   // dados-tecnicos-extraidos.md - lacunas).
//   double pressaoSis1 = SanitizeDouble(pressaoBruta, 0.0, 3200.0, 0.0);
[[nodiscard]] constexpr double SanitizeDouble(
    double val,
    double min,
    double max,
    double fallback
) noexcept {
    if (!(min <= max)) {
        // Faixa mal configurada pelo chamador — não há como sanitizar
        // com segurança, então retorna o fallback declarado.
        return fallback;
    }
    if (!std::isfinite(val)) {
        return fallback;
    }
    if (val < min) {
        return min;
    }
    if (val > max) {
        return max;
    }
    return val;
}

// Variante que não faz clamp silencioso: fora de faixa também vira
// fallback (útil quando "fora de faixa" indica falha de sensor, não um
// valor transitório aceitável a ser limitado).
[[nodiscard]] constexpr double SanitizeDoubleStrict(
    double val,
    double min,
    double max,
    double fallback
) noexcept {
    if (!std::isfinite(val) || val < min || val > max) {
        return fallback;
    }
    return val;
}

// Verifica se um valor é finito e não-NaN — usar antes de qualquer divisão
// ou integração numérica (ex: integração de dt no update() dos sistemas).
[[nodiscard]] constexpr bool IsValidDouble(double val) noexcept {
    return std::isfinite(val);
}

// Divisão segura: evita divisão por zero/valor subnormal, retornando
// fallback em vez de propagar Inf/NaN para o restante do sistema.
[[nodiscard]] constexpr double SafeDivide(
    double numerator,
    double denominator,
    double fallback,
    double epsilon = 1e-9
) noexcept {
    if (!std::isfinite(numerator) || !std::isfinite(denominator)) {
        return fallback;
    }
    if (std::abs(denominator) < epsilon) {
        return fallback;
    }
    return numerator / denominator;
}

// Limita a taxa de variação de um valor entre dois frames (rate limiter).
// Fundamental em sistemas FBW/FADEC: mesmo que a leitura "salte" de um
// frame para outro (glitch de sensor simulado, transição de modo), a saída
// exposta ao resto do avião não pode saltar fisicamente de forma
// instantânea. dt em segundos, maxRatePerSecond na unidade do valor/segundo.
[[nodiscard]] constexpr double RateLimit(
    double previousValue,
    double targetValue,
    double maxRatePerSecond,
    double dt
) noexcept {
    if (!std::isfinite(previousValue) || !std::isfinite(targetValue) ||
        !std::isfinite(dt) || dt <= 0.0 || maxRatePerSecond < 0.0) {
        return previousValue;
    }
    const double maxDelta = maxRatePerSecond * dt;
    const double delta = targetValue - previousValue;
    if (delta > maxDelta) {
        return previousValue + maxDelta;
    }
    if (delta < -maxDelta) {
        return previousValue - maxDelta;
    }
    return targetValue;
}

// dt sanitizado para uso em integração numérica dos sistemas. Um dt
// inválido (0, negativo, NaN, ou um "spike" de frame travado) não pode
// ser integrado diretamente — usar um dt de referência (ex: 1/60s) como
// fallback e um teto para evitar integração explosiva após um frame lento.
[[nodiscard]] constexpr double SanitizeDeltaTime(
    double dt,
    double fallbackDt = 1.0 / 60.0,
    double maxDt = 0.25
) noexcept {
    if (!std::isfinite(dt) || dt <= 0.0) {
        return fallbackDt;
    }
    if (dt > maxDt) {
        return maxDt;
    }
    return dt;
}

} // namespace e195e2::safety
