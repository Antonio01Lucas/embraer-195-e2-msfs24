# Arquitetura

Este documento descreve a organização técnica do projeto e as decisões de
design por trás dela. É a base para qualquer trabalho futuro — antes de
adicionar um sistema, uma tela de aviônico ou um script de build, comece
por aqui.

## Visão geral em camadas

```
┌─────────────────────────────────────────────────────────────┐
│  MSFS 2024 (host)                                            │
│                                                                │
│  ┌──────────────────────┐        ┌───────────────────────┐  │
│  │  Core de Sistemas     │        │  Aviônicos (Coherent   │  │
│  │  (C++20 → WASM)       │◄──────►│  GT / React+TS)        │  │
│  │  src/wasm/            │  dados │  src/avionics/         │  │
│  └──────────────────────┘        └───────────────────────┘  │
│              ▲                                                │
│              │ referenciado por                               │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  Pacote MSFS (aircraft.cfg, panel.xml, ...)           │   │
│  │  PackageSources/SimObjects/Airplanes/embraer-e195-e2  │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

Três camadas independentes, cada uma com sua própria toolchain e suíte de
testes. Nenhuma camada deve depender dos detalhes internos de outra —
a comunicação é sempre por uma interface explícita (ver "Barramento entre
sistemas" abaixo).

## Core de Sistemas (`src/wasm/`)

C++20, compilado como WebAssembly para rodar dentro do MSFS. Cada sistema
físico da aeronave é uma classe que implementa `ISystem`
(`include/ISystem.h`): um único método determinístico,
`update(double dt) noexcept`, chamado a cada frame.

Sistemas implementados hoje:

| Sistema | Arquivo | Responsabilidade |
|---|---|---|
| Elétrico | `ElectricalSystem` | Barramentos AC/DC, geradores (IDG), baterias, load shedding |
| Hidráulico | `HydraulicSystem` | Pressurização por bomba de motor, PTU, despressurização |
| Combustível | `FuelSystem` | Tanques, bombas, crossfeed, balanceamento |
| Pneumático | `PneumaticSystem` | Sangria de motor, crossbleed, carrinho de ar no solo |
| Powerplant | `PowerplantSystem` | N1/N2/Ng, empuxo, resposta ao acelerador (PW1900G) |
| APU | `APUSystem` | Máquina de estados Off → Starting → Running → Stopping |
| Trem de pouso | `LandingGearSystem` | Extensão/retração, extensão de emergência, direção, autobrake |
| Fly-by-wire | `FlyByWire` | Leis normal/direta, proteções (AoA, bank), superfícies |

Todos dependem de `SafetyGuards` (`SanitizeDouble`, `SafeDivide`,
`RateLimit`, `SanitizeDeltaTime`) para nunca propagar `NaN`/`Inf` ou saltos
de estado fisicamente impossíveis — como este código roda a cada frame do
simulador, qualquer valor inválido se propagaria imediatamente para o
resto da aeronave.

**Barramento entre sistemas**: ainda não existe. Hoje cada sistema é
testado isoladamente; a integração entre eles (ex: FBW consumindo pressão
hidráulica real, elétrico alimentando aviônicos) é a próxima peça em
aberto — ver Status abaixo.

### Regras de compilação

`CMakeLists.txt` trata todo warning como erro:
`-Wall -Wextra -Werror -pedantic -Wshadow -Wconversion -Wsign-conversion
-Wnon-virtual-dtor -Woverloaded-virtual`, com ASan/UBSan em build Debug
(em plataformas onde a toolchain os disponibiliza — mingw-w64 no Windows
não empacota `libasan`/`libubsan`; a verificação com sanitizers roda no
CI, que usa Clang no Linux).

### Testes

GoogleTest via `FetchContent` (`tests/CMakeLists.txt`), um arquivo de
teste por sistema. 60 casos cobrindo os 8 sistemas + `SafetyGuards`.

```bash
cmake -S src/wasm -B src/wasm/build -DCMAKE_BUILD_TYPE=Debug -DE195E2_BUILD_TESTS=ON
cmake --build src/wasm/build
ctest --test-dir src/wasm/build --output-on-failure
```

## Aviônicos (`src/avionics/`)

React + TypeScript, buildado com Vite para um bundle único (caminhos
relativos, sem scripts externos — requisito do Coherent GT, o motor de
UI do MSFS). `tsconfig.json` em modo estrito.

Cada Display Unit (DU) do Primus Epic 2.0 é uma página independente
carregada pelo painel do MSFS (`panel/panel.xml` referencia os 5 HTMLs em
`PackageSources/.../panel/`: PFD esquerdo, MFD esquerdo, EICAS, MFD
direito, PFD direito). O código React/TS ainda não está conectado a esses
HTMLs — hoje é um bootstrap mínimo (`src/main.tsx`).

`src/shared/safety.ts` é o espelho em TypeScript das mesmas guardas de
`SafetyGuards.h` do lado C++, para o código de aviônicos que roda fora do
WASM (lógica de UI, formatação de dados de voo) seguir a mesma regra de
nunca propagar valor inválido.

## Pacote MSFS (`PackageSources/`)

Estrutura padrão de um addon MSFS 2024:
`SimObjects/Airplanes/embraer-e195-e2/` com `aircraft.cfg`,
`flight_model.cfg`, `engines.cfg` e `panel/panel.xml`.

`model/` contém um greybox exterior (gerado por
`tools/blender/generate_greybox.py`, exportado para glTF via
`tools/blender/export_greybox_gltf.py`) — geometria de bloqueio de
volume, sem textura, sem interior/cockpit.

**Pendências explícitas** (ver comentários `TODO` nos próprios arquivos,
não preenchidas com números inventados):
- Modelo é só um greybox — sem textura, sem interior/cockpit 3D, sem
  LODs reais.
- Sem `manifest.json`/`layout.json` — conforme a documentação oficial do
  SDK, esses dois arquivos são **gerados** pelo Package Tool/Project
  Editor a partir de `embraer-e195-e2-msfs24.xml` (na raiz do repo), não
  editados à mão. Falta rodar esse projeto pelo SDK real (MSFS 2024
  Developer Mode ou Package Tool standalone) — não disponível no
  ambiente onde este repo foi desenvolvido até agora.
- CG, V-speeds de baixa velocidade, geometria detalhada de asa
  (chord/dihedral/sweep) e curvas reais de FADEC ainda não têm fonte
  confirmada.
- Binding das 5 telas do painel (`SCREEN_DU*` no greybox) ao sistema de
  glasscockpit do MSFS 2024 SDK ainda não resolvido — não incluídas no
  export atual do modelo por esse motivo.

## CI (`.github/workflows/security-audit.yml`)

Dois jobs, em toda PR/push para `main`:
1. **Audit + Lint**: `scripts/audit.js`, type-check estrito do TypeScript,
   ESLint.
2. **Build de verificação do core WASM**: compila o core como biblioteca
   nativa (não WASM real) com Clang + sanitizers no Linux, só para validar
   que as flags estritas passam rápido em CI. A suíte GoogleTest também
   roda aqui (`-DE195E2_BUILD_TESTS=ON` + `ctest`). A compilação real para
   WebAssembly usa a toolchain do MSFS SDK, que roda localmente na máquina
   Windows do desenvolvedor.

## Princípios de projeto

1. **Nenhum dado físico fabricado.** Números que não têm fonte confirmada
   ficam como `TODO` explícito no arquivo, nunca como um valor plausível
   inventado.
2. **Nenhum warning silencioso.** `-Werror` em todo o core C++; TypeScript
   em modo estrito.
3. **Sanitização de fronteira.** Todo valor que entra em um sistema físico
   (dt, comandos externos) passa por `SafetyGuards` antes de ser usado.
4. **Cada camada testa a si mesma.** GoogleTest para o core C++, Vitest
   para o TypeScript compartilhado — sem depender de rodar o simulador
   para validar lógica.
