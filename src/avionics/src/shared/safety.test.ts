import { describe, expect, it } from "vitest";
import {
  rateLimit,
  safeDivide,
  sanitizeDeltaTimeSeconds,
  sanitizeNumber,
  sanitizeNumberStrict,
} from "./safety";

describe("sanitizeNumber", () => {
  it("passa o valor quando está dentro da faixa", () => {
    expect(sanitizeNumber(50, 0, 100, -1)).toBe(50);
  });

  it("satura no mínimo quando abaixo da faixa", () => {
    expect(sanitizeNumber(-10, 0, 100, -1)).toBe(0);
  });

  it("satura no máximo quando acima da faixa", () => {
    expect(sanitizeNumber(150, 0, 100, -1)).toBe(100);
  });

  it("retorna fallback para NaN", () => {
    expect(sanitizeNumber(NaN, 0, 100, -1)).toBe(-1);
  });

  it("retorna fallback para +/-Infinity", () => {
    expect(sanitizeNumber(Infinity, 0, 100, -1)).toBe(-1);
    expect(sanitizeNumber(-Infinity, 0, 100, -1)).toBe(-1);
  });

  it("retorna fallback quando a faixa está mal configurada (min > max)", () => {
    expect(sanitizeNumber(50, 100, 0, -1)).toBe(-1);
  });
});

describe("sanitizeNumberStrict", () => {
  it("retorna fallback (não satura) quando fora da faixa", () => {
    expect(sanitizeNumberStrict(150, 0, 100, -1)).toBe(-1);
  });

  it("passa o valor quando dentro da faixa", () => {
    expect(sanitizeNumberStrict(50, 0, 100, -1)).toBe(50);
  });
});

describe("safeDivide", () => {
  it("divide normalmente quando o denominador é seguro", () => {
    expect(safeDivide(10, 2, -1)).toBe(5);
  });

  it("retorna fallback quando o denominador é ~0", () => {
    expect(safeDivide(10, 1e-12, -1)).toBe(-1);
  });

  it("retorna fallback quando algum operando é NaN", () => {
    expect(safeDivide(NaN, 2, -1)).toBe(-1);
  });
});

describe("rateLimit", () => {
  it("limita o avanço positivo pela taxa máxima", () => {
    expect(rateLimit(0, 100, 10, 1)).toBe(10);
  });

  it("limita o avanço negativo pela taxa máxima", () => {
    expect(rateLimit(100, 0, 10, 1)).toBe(90);
  });

  it("passa o valor alvo quando o delta é menor que o máximo permitido", () => {
    expect(rateLimit(50, 55, 100, 1)).toBe(55);
  });

  it("retorna o valor anterior quando dt é inválido", () => {
    expect(rateLimit(50, 100, 10, -1)).toBe(50);
    expect(rateLimit(50, 100, 10, 0)).toBe(50);
  });
});

describe("sanitizeDeltaTimeSeconds", () => {
  it("passa dt normal sem alteração", () => {
    expect(sanitizeDeltaTimeSeconds(1 / 60)).toBeCloseTo(1 / 60);
  });

  it("retorna fallback para dt <= 0", () => {
    expect(sanitizeDeltaTimeSeconds(0, 0.5)).toBe(0.5);
    expect(sanitizeDeltaTimeSeconds(-1, 0.5)).toBe(0.5);
  });

  it("satura dt excessivamente grande no teto configurado", () => {
    expect(sanitizeDeltaTimeSeconds(10, 1 / 60, 0.25)).toBe(0.25);
  });
});
