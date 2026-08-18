"""Gera o greybox (mesh de referencia) do Embraer E195-E2 no Blender via bpy.

Ref: Regra de Conduta do Projeto #1 ("Fisica em Primeiro Lugar") - as mesmas
regras que valem para flight_model.cfg valem aqui: nenhuma dimensao e
inventada sem base. Dimensoes principais vem de
PackageSources/.../flight_model.cfg (cross-check Embraer/doc8643/Wikipedia,
ja documentado la). Tudo que NAO tem fonte confirmada (diametro de
fuselagem, forma real da asa/enflechamento/diedro, posicao de nacele,
pontos de trem de pouso, ergonomia do painel) e um PLACEHOLDER geometrico
explicito, marcado como tal abaixo - nao um "chute" disfarcado de dado
real. Ajuste visualmente no Blender / substitua por dados de 3-view oficial
antes de qualquer uso alem de bloqueio de volume e teste de camera.

Como rodar:
    1. Blender > Scripting tab > abrir este arquivo > Run Script; ou
    2. Linha de comando (headless):
       blender --background --python tools/blender/generate_greybox.py

Compatibilidade: Blender 3.6+ / 4.x (API bpy padrao, sem addons externos).

Depois de gerar:
    - Exportar via addon oficial "Microsoft Flight Simulator" (glTF) do
      Blender. CONFIRME a convencao de eixos do exportador (forward/up)
      antes de exportar - este script assume +Y = nariz, +Z = cima,
      +X = direita (padrao Blender), mas o addon MSFS pode exigir conversao.
    - As 5 telas (SCREEN_DU*) precisam do material/template de glasscockpit
      do MSFS 2024 SDK vinculado ao Panel correspondente em panel.xml
      (Panel_DU1_PFD_L .. Panel_DU5_PFD_R) - o binding exato (nome de
      material / template de comportamento) varia por versao do SDK;
      validar contra a documentacao oficial atual antes de compilar o
      pacote (mesma ressalva ja registrada em panel.xml).
"""

import bpy

# --------------------------------------------------------------------------
# Dimensoes CONFIRMADAS - fonte: PackageSources/.../flight_model.cfg
# (cross-check folheto oficial Embraer "Profit Hunter" + doc8643.com +
# Wikipedia, ja documentado nos comentarios daquele arquivo).
# --------------------------------------------------------------------------
WING_SPAN_M = 35.12          # wing_span = 115.25 ft
WING_AREA_M2 = 103.0         # wing_area = 1110.0 ft2
FUSELAGE_LENGTH_M = 41.51    # fuselage_length = 136.17 ft
AIRCRAFT_TOTAL_HEIGHT_M = 10.91  # fuselage_height = 35.83 ft (altura TOTAL
                                   # da aeronave incluindo empenagem, nao a
                                   # altura da secao de fuselagem - mesma
                                   # ressalva do flight_model.cfg)

# Corda media derivada matematicamente das duas confirmadas acima
# (area / envergadura) - nao e um valor novo inventado, e aritmetica sobre
# dado ja cross-checado.
WING_AVERAGE_CHORD_M = WING_AREA_M2 / WING_SPAN_M  # ~2.93 m

# --------------------------------------------------------------------------
# PLACEHOLDERS - sem fonte confirmada. Cada um comentado com o motivo.
# Nao usar estes valores para dados aerodinamicos reais (aquilo continua
# TODO em flight_model.cfg); servem só para o greybox ter volume e
# proporcao plausivel para bloqueio de camera / teste de escala.
# --------------------------------------------------------------------------

# Diametro de fuselagem: ESTIMATIVA DE ENGENHARIA via razao L/D tipica de
# jatos regionais de fuselagem estreita (~12-13:1), mesmo espirito do
# metodo Roskam ja usado para radius_of_gyration em flight_model.cfg.
# TODO(Fase 4): substituir por secao transversal real (desenho 3-view
# oficial) assim que disponivel.
_FUSELAGE_LD_RATIO_ESTIMATE = 12.5
FUSELAGE_DIAMETER_M = FUSELAGE_LENGTH_M / _FUSELAGE_LD_RATIO_ESTIMATE  # ~3.32 m

# Asa modelada SEM enflechamento/diedro de proposito: flight_model.cfg
# mantem wing_sweep=0.0 e wing_dihedral=0.0 como TODO explicito (dado nao
# confirmado publicamente) - o greybox reflete esse mesmo "nao sabemos
# ainda" em vez de desenhar um angulo bonito porem fabricado.
WING_THICKNESS_M = 0.30              # placeholder, proporcao arbitraria
WING_ROOT_STATION_FRACTION = 0.42    # posicao longitudinal da raiz da asa
                                       # como fracao do comprimento da
                                       # fuselagem - placeholder arbitrario

# Empenagem: proporcoes arbitrarias (fracao de span/comprimento ja
# confirmados), sem fonte especifica - so para o greybox nao ficar sem
# cauda. TODO(Fase 4): refinar com 3-view oficial.
HSTAB_SPAN_M = WING_SPAN_M * 0.34
HSTAB_CHORD_M = WING_AVERAGE_CHORD_M * 0.55
HSTAB_THICKNESS_M = 0.22
VSTAB_HEIGHT_M = FUSELAGE_LENGTH_M * 0.135
VSTAB_CHORD_M = WING_AVERAGE_CHORD_M * 0.75
VSTAB_THICKNESS_M = 0.24
TAIL_STATION_FRACTION = 0.94  # posicao longitudinal da empenagem

# Naceles do motor (PW1900G, ver engines.cfg) - instalacao subalar tipica
# desta classe de aeronave. Diametro/posicao SEM fonte confirmada nesta
# sessao - placeholder. TODO(Fase 4): confirmar estacao real da nacele.
NACELLE_DIAMETER_M = 1.9
NACELLE_LENGTH_M = 3.4
NACELLE_SPAN_STATION_FRACTION = 0.33  # fracao da semi-envergadura
NACELLE_DROP_M = 1.1                  # abaixo do plano da asa

# Pontos de trem de pouso: apenas marcadores (Empty), nao geometria real.
# Ajudam a preencher [CONTACT_POINTS] (pendente em flight_model.cfg) mais
# tarde - posicoes sao placeholder.
NOSE_GEAR_STATION_FRACTION = 0.12
MAIN_GEAR_STATION_FRACTION = 0.48
MAIN_GEAR_TRACK_M = 5.0  # distancia entre as duas pernas principais

# Telas dos 5 DUs (Primus Epic 2.0) - nomes alinhados com panel.xml
# (Panel_DU1_PFD_L .. Panel_DU5_PFD_R). Tamanho/posicao sao placeholder de
# ergonomia de cockpit - TODO(Fase 4): validar contra referencia real do
# painel Primus Epic 2.0.
DU_SCREEN_WIDTH_M = 0.26
DU_SCREEN_HEIGHT_M = 0.19
DU_PANEL_STATION_FRACTION = 0.085  # bem perto do nariz
DU_PANEL_HEIGHT_M = 1.25
DU_NAMES = [
    "SCREEN_DU1_PFD_L",
    "SCREEN_DU2_MFD_L",
    "SCREEN_DU3_EICAS",
    "SCREEN_DU4_MFD_R",
    "SCREEN_DU5_PFD_R",
]


def clear_scene() -> None:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)


def get_or_create_collection(name: str) -> bpy.types.Collection:
    if name in bpy.data.collections:
        return bpy.data.collections[name]
    collection = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(collection)
    return collection


def _link_only_to(obj: bpy.types.Object, collection: bpy.types.Collection) -> None:
    for coll in list(obj.users_collection):
        coll.objects.unlink(obj)
    collection.objects.link(obj)


def add_box(name: str, size_xyz, location_xyz, collection: bpy.types.Collection) -> bpy.types.Object:
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=location_xyz)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = (size_xyz[0] / 2.0, size_xyz[1] / 2.0, size_xyz[2] / 2.0)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    _link_only_to(obj, collection)
    return obj


def add_cylinder_along_y(name: str, length_m: float, radius_m: float, location_xyz, collection: bpy.types.Collection) -> bpy.types.Object:
    bpy.ops.mesh.primitive_cylinder_add(
        radius=radius_m,
        depth=length_m,
        location=location_xyz,
        rotation=(1.5707963, 0.0, 0.0),  # 90 graus em X: eixo do cilindro Z -> Y
    )
    obj = bpy.context.active_object
    obj.name = name
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
    _link_only_to(obj, collection)
    return obj


def add_empty(name: str, location_xyz, collection: bpy.types.Collection) -> bpy.types.Object:
    bpy.ops.object.empty_add(type="PLAIN_AXES", radius=0.3, location=location_xyz)
    obj = bpy.context.active_object
    obj.name = name
    _link_only_to(obj, collection)
    return obj


def add_screen_plane(name: str, width_m: float, height_m: float, location_xyz, collection: bpy.types.Collection) -> bpy.types.Object:
    bpy.ops.mesh.primitive_plane_add(size=1.0, location=location_xyz)
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = (width_m / 2.0, height_m / 2.0, 1.0)
    # Vira a placa para encarar +Y (para frente do avião, na direção do
    # piloto) - rotaciona 90 graus em X a partir do plano padrão (que fica
    # deitado no plano XY).
    obj.rotation_euler = (1.5707963, 0.0, 0.0)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    _link_only_to(obj, collection)
    return obj


def make_fuselage(collection: bpy.types.Collection) -> None:
    add_cylinder_along_y(
        "GREYBOX_Fuselage",
        length_m=FUSELAGE_LENGTH_M,
        radius_m=FUSELAGE_DIAMETER_M / 2.0,
        location_xyz=(0.0, FUSELAGE_LENGTH_M / 2.0, 0.0),
        collection=collection,
    )


def make_wings(collection: bpy.types.Collection) -> None:
    half_span = WING_SPAN_M / 2.0
    root_y = FUSELAGE_LENGTH_M * WING_ROOT_STATION_FRACTION
    fuselage_radius = FUSELAGE_DIAMETER_M / 2.0
    wing_span_extent = half_span - fuselage_radius

    for side, sign in (("L", -1.0), ("R", 1.0)):
        x_center = sign * (fuselage_radius + wing_span_extent / 2.0)
        add_box(
            f"GREYBOX_Wing_{side}",
            size_xyz=(wing_span_extent, WING_AVERAGE_CHORD_M, WING_THICKNESS_M),
            location_xyz=(x_center, root_y, 0.0),
            collection=collection,
        )


def make_stabilizers(collection: bpy.types.Collection) -> None:
    tail_y = FUSELAGE_LENGTH_M * TAIL_STATION_FRACTION
    fuselage_radius = FUSELAGE_DIAMETER_M / 2.0
    half_hstab = HSTAB_SPAN_M / 2.0
    hstab_extent = half_hstab - fuselage_radius

    for side, sign in (("L", -1.0), ("R", 1.0)):
        x_center = sign * (fuselage_radius + hstab_extent / 2.0)
        add_box(
            f"GREYBOX_HStab_{side}",
            size_xyz=(hstab_extent, HSTAB_CHORD_M, HSTAB_THICKNESS_M),
            location_xyz=(x_center, tail_y, AIRCRAFT_TOTAL_HEIGHT_M * 0.05),
            collection=collection,
        )

    add_box(
        "GREYBOX_VStab",
        size_xyz=(VSTAB_THICKNESS_M, VSTAB_CHORD_M, VSTAB_HEIGHT_M),
        location_xyz=(0.0, tail_y, VSTAB_HEIGHT_M / 2.0 + fuselage_radius * 0.5),
        collection=collection,
    )


def make_nacelles(collection: bpy.types.Collection) -> None:
    wing_root_y = FUSELAGE_LENGTH_M * WING_ROOT_STATION_FRACTION
    half_span = WING_SPAN_M / 2.0

    for side, sign in (("L", -1.0), ("R", 1.0)):
        x_center = sign * half_span * NACELLE_SPAN_STATION_FRACTION
        add_cylinder_along_y(
            f"GREYBOX_Nacelle_{side}",
            length_m=NACELLE_LENGTH_M,
            radius_m=NACELLE_DIAMETER_M / 2.0,
            location_xyz=(x_center, wing_root_y, -NACELLE_DROP_M),
            collection=collection,
        )


def make_gear_markers(collection: bpy.types.Collection) -> None:
    ground_z = -AIRCRAFT_TOTAL_HEIGHT_M * 0.42  # placeholder de altura de solo
    nose_y = FUSELAGE_LENGTH_M * NOSE_GEAR_STATION_FRACTION
    main_y = FUSELAGE_LENGTH_M * MAIN_GEAR_STATION_FRACTION

    add_empty("CONTACT_NoseGear", (0.0, nose_y, ground_z), collection)
    add_empty("CONTACT_MainGear_L", (-MAIN_GEAR_TRACK_M / 2.0, main_y, ground_z), collection)
    add_empty("CONTACT_MainGear_R", (MAIN_GEAR_TRACK_M / 2.0, main_y, ground_z), collection)


def make_du_screens(collection: bpy.types.Collection) -> None:
    panel_y = FUSELAGE_LENGTH_M * DU_PANEL_STATION_FRACTION
    spacing = DU_SCREEN_WIDTH_M * 1.15
    total_width = spacing * (len(DU_NAMES) - 1)
    start_x = -total_width / 2.0

    for i, name in enumerate(DU_NAMES):
        x = start_x + i * spacing
        add_screen_plane(
            name,
            width_m=DU_SCREEN_WIDTH_M,
            height_m=DU_SCREEN_HEIGHT_M,
            location_xyz=(x, panel_y, DU_PANEL_HEIGHT_M),
            collection=collection,
        )


def main() -> None:
    clear_scene()
    collection = get_or_create_collection("E195E2_Greybox")

    make_fuselage(collection)
    make_wings(collection)
    make_stabilizers(collection)
    make_nacelles(collection)
    make_gear_markers(collection)
    make_du_screens(collection)

    created = [obj.name for obj in collection.objects]
    print(f"[generate_greybox] {len(created)} objetos criados em '{collection.name}':")
    for obj_name in created:
        print(f"  - {obj_name}")
    print(
        "[generate_greybox] Lembrete: dimensoes marcadas como PLACEHOLDER no "
        "topo do script nao tem fonte confirmada - ver comentarios antes de "
        "usar para algo alem de bloqueio de volume/camera."
    )


if __name__ == "__main__":
    main()
