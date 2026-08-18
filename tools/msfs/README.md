# Ponte SimBus → L:Vars (MSFS 2024 Vars API)

Publica o estado do `SimBus` (elétrico, trem de pouso, flap/slat/
speedbrake/spoilers via `FlyByWire`) como L:Vars, para o React dos
aviônicos (`src/avionics`) ler via `SimVar.GetSimVarValue("L:<nome>", "number")`.

## Duas camadas, propósito diferente

- **`src/wasm/include/SimVarPublisher.h` / `.cpp`** — PURA, sem
  dependência do SDK do MSFS. Traduz `SimBus` para a lista de 20
  `{nome, unidade, valor}` a publicar. Testada de verdade com GoogleTest
  (`SimVarPublisherTests.cpp`), faz parte do build normal
  (`E195E2_BUILD_TESTS=ON`).
- **`tools/msfs/lvar_bridge.h` / `.cpp`** — chama a Vars API real do MSFS
  2024 (`fsVarsRegisterLVar`, `fsVarsGetUnitId`, `fsVarsLVarSet`) para de
  fato criar/atualizar os L:Vars no sim. **Não compilado neste
  ambiente** (sem os headers do MSFS 2024 WASM SDK instalados) e **não**
  incluído em `CMakeLists.txt` de propósito - escrito a partir da
  documentação oficial (fontes citadas no topo do `.h`), mas sem
  verificação de compilação real. Validar contra o SDK instalado antes
  de usar em produção.

## L:Vars publicados hoje (prefixo `E195E2_`)

Elétrico (10): `ELEC_AC1_POWERED`, `ELEC_AC2_POWERED`,
`ELEC_AC_ESSENTIAL_POWERED`, `ELEC_DC1_POWERED`, `ELEC_DC2_POWERED`,
`ELEC_DC_ESSENTIAL_POWERED`, `ELEC_DC_EMERGENCY_POWERED`,
`ELEC_LOAD_SHEDDING_ACTIVE`, `ELEC_BATTERY_1_PERCENT`,
`ELEC_BATTERY_2_PERCENT`.

Trem de pouso (4): `GEAR_NOSE_POSITION`, `GEAR_LEFT_MAIN_POSITION`,
`GEAR_RIGHT_MAIN_POSITION` (0=Up, 1=InTransit, 2=Down),
`GEAR_NOSEWHEEL_STEERING_DEG`.

Fly-by-wire (6): `FBW_FLAP_PERCENT`, `FBW_SLAT_PERCENT`,
`FBW_SPEEDBRAKE_PERCENT`, `FBW_GROUND_SPOILER_PERCENT`,
`FBW_ROLL_SPOILER_PERCENT`, `FBW_CONTROL_LAW` (0=Normal, 1=Direct).

Hidráulico, motor/APU e combustível ficam para uma leva seguinte, mesmo
padrão (adicionar ao `SimVarPublisher::collect()` e aos testes).

## O que falta para isto rodar de verdade no sim

1. Confirmar o caminho do `#include` da Vars API contra o SDK instalado
   (não encontrado na documentação consultada - ver TODO em
   `lvar_bridge.h`).
2. Confirmar os nomes/assinaturas reais dos callbacks de ciclo de vida do
   módulo WASM (init/update) do MSFS 2024 e chamar
   `LVarBridge::registerAll()`/`publish()` a partir deles.
3. Compilar de fato para WebAssembly com o toolchain do MSFS SDK (fora do
   CMake atual, que compila o core como biblioteca nativa para testes).
