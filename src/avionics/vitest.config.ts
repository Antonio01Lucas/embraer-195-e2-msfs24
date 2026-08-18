// Ref: Regra de Conduta do Projeto - build seguro e verificável.
//
// Config separada de vite.config.ts (que é dedicada ao bundle único de
// produção para o Coherent GT, com vite-plugin-singlefile). Testes rodam
// em Node puro — não precisam do plugin de single-file nem de um DOM real
// para o utilitário compartilhado em src/shared (funções puras).
import { defineConfig } from "vitest/config";

export default defineConfig({
  test: {
    environment: "node",
    include: ["src/**/*.test.ts", "src/**/*.test.tsx"],
    coverage: {
      provider: "v8",
      reporter: ["text", "html"],
    },
  },
});
