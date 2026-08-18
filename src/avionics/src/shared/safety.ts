// Ref: Regra de Conduta do Projeto #1 - "Física em Primeiro Lugar"
//
// Equivalente em TypeScript de src/wasm/include/SafetyGuards.h. Os DUs
// (PFD/MFD/EICAS) leem SimVars/LVars vindos do motor do MSFS e do core em
// WASM — esses valores podem chegar como NaN/undefined/Infinity (variável
// não inicializada, transição de estado, glitch de leitura). Toda leitura
// bruta de SimVar deve passar por aqui antes de alimentar um componente
// React, para os displays nunca renderizarem lixo numérico.

/**
 * Sanitiza um número contra NaN, +/-Infinity e fora de faixa fisicamente
 * válida, retornando um fallback seguro nesses casos.
 *
 * Uso típico:
 *   // Ref: ATA 29 (Hydraulic) - faixa nominal do Sistema 1: 0 a 3000 PSI.
 *   const pressaoSis1 = sanitizeNumber(rawSimVar, 0, 3200, 0);
 */
export function sanitizeNumber(
  value: number,
  min: number,
  max: number,
  fallback: number,
): number {
  if (!(min <= max)) {
    return fallback;
  }
  if (!Number.isFinite(value)) {
    return fallback;
  }
  if (value < min) {
    return min;
  }
  if (value > max) {
    return max;
  }
  return value;
}

/** Variante estrita: fora de faixa também retorna fallback (não faz clamp). */
export function sanitizeNumberStrict(
  value: number,
  min: number,
  max: number,
  fallback: number,
): number {
  if (!Number.isFinite(value) || value < min || value > max) {
    return fallback;
  }
  return value;
}

/** Divisão segura: evita divisão por zero/valor subnormal. */
export function safeDivide(
  numerator: number,
  denominator: number,
  fallback: number,
  epsilon = 1e-9,
): number {
  if (!Number.isFinite(numerator) || !Number.isFinite(denominator)) {
    return fallback;
  }
  if (Math.abs(denominator) < epsilon) {
    return fallback;
  }
  return numerator / denominator;
}

/**
 * Limita a taxa de variação de um valor entre dois frames de renderização
 * (rate limiter) — equivalente de e195e2::safety::RateLimit em
 * SafetyGuards.h. Útil para animações de ponteiro/fita que não devem
 * "saltar" instantaneamente mesmo que o SimVar de origem salte.
 */
export function rateLimit(
  previousValue: number,
  targetValue: number,
  maxRatePerSecond: number,
  dtSeconds: number,
): number {
  if (
    !Number.isFinite(previousValue) ||
    !Number.isFinite(targetValue) ||
    !Number.isFinite(dtSeconds) ||
    dtSeconds <= 0 ||
    maxRatePerSecond < 0
  ) {
    return previousValue;
  }
  const maxDelta = maxRatePerSecond * dtSeconds;
  const delta = targetValue - previousValue;
  if (delta > maxDelta) {
    return previousValue + maxDelta;
  }
  if (delta < -maxDelta) {
    return previousValue - maxDelta;
  }
  return targetValue;
}

/**
 * dt sanitizado para uso em animações/integração no lado dos aviônicos.
 * Mesmo racional de SafetyGuards::SanitizeDeltaTime: um dt inválido ou um
 * "spike" de frame travado do Coherent GT não pode ser usado diretamente.
 */
export function sanitizeDeltaTimeSeconds(
  dtSeconds: number,
  fallbackDt = 1 / 60,
  maxDt = 0.25,
): number {
  if (!Number.isFinite(dtSeconds) || dtSeconds <= 0) {
    return fallbackDt;
  }
  if (dtSeconds > maxDt) {
    return maxDt;
  }
  return dtSeconds;
}
