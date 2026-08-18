# Greybox generator

`generate_greybox.py` cria, via `bpy`, um mesh de referência (fuselagem,
asas, empenagem, naceles, marcadores de trem de pouso e as 5 telas dos DUs
do Primus Epic 2.0) para bloqueio de volume e teste de câmera no MSFS —
**não** é o modelo 3D final.

## Rodar

No Blender (3.6+/4.x, testado em 5.2 LTS):

```
Scripting tab > abrir generate_greybox.py > Run Script
```

Ou headless:

```bash
blender --background --python tools/blender/generate_greybox.py
```

Cria uma collection `E195E2_Greybox` com todos os objetos nomeados.

## Sobre as dimensões

O topo do arquivo separa explicitamente:
- **Confirmadas**: `WING_SPAN_M`, `WING_AREA_M2`, `FUSELAGE_LENGTH_M`,
  `AIRCRAFT_TOTAL_HEIGHT_M` — vêm de
  `PackageSources/.../flight_model.cfg` (já cross-checado lá).
- **Placeholder/estimativa**: tudo que não tem fonte confirmada (diâmetro
  de fuselagem, posição/tamanho de empenagem e naceles, ergonomia do
  painel). Cada uma comenta o porquê. Não usar esses números como se
  fossem dado real — mesma regra do resto do projeto.

## Exportar

`export_greybox_gltf.py` já faz isso: gera o greybox, seleciona só os
objetos `GREYBOX_*` (exterior) e exporta via exportador glTF nativo do
Blender para
`PackageSources/SimObjects/Airplanes/embraer-e195-e2/model/embraer_e195e2_greybox.gltf`.

```bash
blender --background --python tools/blender/export_greybox_gltf.py
```

Verificado de verdade: rodado headless no Blender 5.2 LTS local, com
reimport do `.gltf` gerado confirmando os 8 objetos exportados.

As telas (`SCREEN_DU*`) e os marcadores de trem de pouso (`CONTACT_*`)
ficam de fora desse export de propósito — telas porque o binding de
glasscockpit do MSFS 2024 SDK ainda não foi resolvido (ver
`docs/ARCHITECTURE.md`), marcadores porque são só referência visual para
preencher `[CONTACT_POINTS]` em `flight_model.cfg`, não geometria real.

Este export usa o exportador glTF **nativo** do Blender (sem addon
externo) — suficiente para um greybox estático. Animações/comportamentos
interativos (trem de pouso, superfícies de comando) exigem o addon
oficial "Microsoft Flight Simulator" do Blender, que embute extensões
específicas do SDK no glTF — ainda não instalado neste projeto.
