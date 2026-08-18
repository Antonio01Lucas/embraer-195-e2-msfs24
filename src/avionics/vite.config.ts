// Ref: MSFS 2024 SDK - Coherent GT Panel Rendering
//
// Coherent GT (o motor de renderização de painéis do MSFS) carrega o painel
// como um documento HTML isolado, sem acesso a rede externa e sem injeção de
// scripts de fora do pacote. Por isso o bundle precisa ser:
//   1) único arquivo autocontido (JS/CSS inlined) — nada de <script src="https://...">;
//   2) referenciado por caminhos relativos (o painel é copiado para dentro do
//      pacote MSFS, caminhos absolutos do host de build não fazem sentido);
//   3) sem "code splitting" — Coherent GT não resolve chunks dinâmicos como um
//      browser moderno faria.
import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import { viteSingleFile } from "vite-plugin-singlefile";

export default defineConfig({
  // Caminhos relativos: o bundle final é copiado para dentro de
  // PackageSources/SimObjects/Airplanes/embraer-e195-e2/panel/, então uma
  // base absoluta ("/") quebraria a resolução de assets dentro do MSFS.
  base: "./",

  plugins: [
    react(),
    // Garante saída em arquivo único (JS + CSS inlined no HTML) — requisito
    // do Coherent GT, que não faz fetch de chunks adicionais em runtime.
    viteSingleFile(),
  ],

  build: {
    outDir: "dist",
    emptyOutDir: true,
    // Sem code-splitting: um único entry point, um único bundle.
    target: "es2020",
    cssCodeSplit: false,
    assetsInlineLimit: 100_000_000, // força inline de todos os assets (fonts/svg/etc.)
    rollupOptions: {
      input: "index.html",
      output: {
        // Sem hashing de nome de arquivo — facilita referenciar o bundle
        // de forma determinística a partir do panel.xml.
        entryFileNames: "e195e2-avionics.js",
        assetFileNames: "e195e2-avionics.[ext]",
      },
    },
    // Nenhuma dependência deve ser resolvida via CDN externo em runtime —
    // tudo precisa estar empacotado dentro do bundle único.
    minify: "esbuild",
  },

  server: {
    // Apenas para desenvolvimento local fora do MSFS (preview em navegador).
    port: 5173,
    strictPort: true,
  },
});
