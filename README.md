# Embraer E195-E2 para Microsoft Flight Simulator 2024

[![Security & Integrity Audit](https://github.com/Antonio01Lucas/embraer-195-e2-msfs24/actions/workflows/security-audit.yml/badge.svg)](https://github.com/Antonio01Lucas/embraer-195-e2-msfs24/actions/workflows/security-audit.yml)

Addon *study-level* do Embraer E195-E2 para o MSFS 2024. Projeto
independente de fã — **sem afiliação oficial com a Embraer S.A.**

## Status

Fase de fundação: o core de sistemas físicos, o greybox 3D e a estrutura
do pacote MSFS existem e são testados; ainda **não carrega no simulador**
(falta gerar `manifest.json`/`layout.json` via Package Tool — ver
[Pendências](#pendências)).

| Camada | Estado |
|---|---|
| Core de sistemas (C++20/WASM) | 8 sistemas + `SimBus` + `SimVarPublisher`, **71/71 testes passando** (GoogleTest) |
| Aviônicos (React/TS) | Scaffold + guardas de segurança compartilhadas (Vitest) |
| Modelo 3D | Greybox exterior (glTF) gerado via Blender, sem textura/interior |
| Pacote MSFS | `aircraft.cfg`/`flight_model.cfg`/`engines.cfg`/`panel.xml`/`model/` presentes; falta gerar manifest/layout |
| CI | Audit + lint + build de verificação com sanitizers em toda PR |

Detalhes de design em [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Estrutura

```
├── src/
│   ├── wasm/           # Core de sistemas físicos (C++20 → WebAssembly)
│   │   ├── include/    # ISystem + 8 sistemas + SafetyGuards
│   │   ├── src/        # Implementações
│   │   └── tests/      # Suíte GoogleTest (60 casos)
│   └── avionics/       # Aviônicos Primus Epic 2.0 (React + TypeScript + Vite)
├── PackageSources/
│   └── SimObjects/Airplanes/embraer-e195-e2/   # Pacote MSFS 2024
├── scripts/audit.js    # Auditoria de segurança/integridade da estrutura
├── docs/ARCHITECTURE.md
└── .github/workflows/security-audit.yml
```

## Requisitos

- Node.js 20+
- CMake 3.20+
- Compilador C++20 (GCC 11+, Clang 14+, ou MSVC 2022) — **não** o MinGW
  antigo (GCC ≤ 6) às vezes presente por padrão no Windows, ele não
  suporta C++20.

## Build e testes

```bash
npm install
npm --workspace=src/avionics install

npm run build     # WASM (Release) + aviônicos
npm run test      # GoogleTest (wasm) + Vitest (avionics)
npm run audit      # auditoria de segurança/integridade da estrutura
```

Só o core C++, com testes e sanitizers (Debug):

```bash
cmake -S src/wasm -B src/wasm/build -DCMAKE_BUILD_TYPE=Debug -DE195E2_BUILD_TESTS=ON
cmake --build src/wasm/build
ctest --test-dir src/wasm/build --output-on-failure
```

> No Windows, ASan/UBSan exigem uma toolchain que empacote
> `libasan`/`libubsan` (ex.: MSYS2/mingw-w64 recente ou Clang com
> compiler-rt) — o MinGW.org clássico não os inclui. Sem sanitizers,
> use `-DCMAKE_BUILD_TYPE=RelWithDebInfo` no lugar de `Debug`.

## Dados técnicos

Dimensões, pesos e desempenho vêm de fontes públicas cross-checadas
(folheto oficial Embraer "Profit Hunter", doc8643.com, FAA Order
JO 7360.1E) e são citados nos comentários dos próprios arquivos de
configuração (`aircraft.cfg`, `flight_model.cfg`, `engines.cfg`).

Regra do projeto: **nenhum dado físico é inventado**. Onde não há fonte
confirmada (CG, V-speeds de baixa velocidade, geometria detalhada de asa,
curvas de FADEC), o campo fica como `TODO` explícito no arquivo em vez de
receber um número plausível, porém fabricado.

## Pendências

- Gerar `manifest.json`/`layout.json` a partir de
  `embraer-e195-e2-msfs24.xml` pelo Package Tool/Project Editor do SDK
  (MSFS 2024 Developer Mode) — sem isso o addon não é reconhecido pelo
  simulador. Não é para editar esses dois arquivos à mão (a própria
  documentação do SDK avisa que isso desatualiza o índice do pacote).
- Modelo 3D é só um greybox — sem textura, sem interior/cockpit real.
- Acoplamentos ainda não ligáveis no `SimBus` (combustível ↔ powerplant,
  pneumático ↔ motor/APU, sensores de atitude → FBW) — bloqueados por
  lacuna de API no sistema de origem/destino, ver `SimBus.h`.
- Conectar o React dos aviônicos aos 5 HTMLs de DU do painel (depende do
  modelo de interior e do binding de glasscockpit, ainda não resolvido) —
  já tem as L:Vars publicadas (ver `tools/msfs/README.md`) para ler via
  `SimVar.GetSimVarValue`, falta o lado React consumir.
- `tools/msfs/lvar_bridge.*` (chamada real à Vars API do MSFS 2024) não
  está compilada/testada neste ambiente — falta o SDK real. Ver
  `tools/msfs/README.md` para o que falta confirmar.

## Licença

Código sob [MIT](LICENSE). Isso cobre o código-fonte (C++, TypeScript,
configs) — não implica nenhum direito sobre as marcas "Embraer" ou
"Profit Hunter", que pertencem à Embraer S.A. (ver Aviso legal).

## Contribuindo

Antes de tocar em qualquer sistema físico ou na estrutura do pacote, leia
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — em especial os princípios
de projeto (sem dados fabricados, `-Werror` em tudo, sanitização de
fronteira).

## Aviso legal

Projeto de fã, sem fins comerciais, sem afiliação com a Embraer S.A. Todas
as marcas e nomes de produtos citados pertencem aos seus respectivos
donos.
