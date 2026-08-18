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

## Depois de gerar

Exportar via addon oficial "Microsoft Flight Simulator" do Blender.
Confirme a convenção de eixos do exportador antes de exportar (o script
assume +Y = nariz, +Z = cima, +X = direita). As 5 telas (`SCREEN_DU*`)
ainda precisam do material/template de glasscockpit do MSFS 2024 SDK
vinculado ao `Panel` correspondente em `panel.xml` — o binding exato
varia por versão do SDK, validar contra a documentação oficial atual.
