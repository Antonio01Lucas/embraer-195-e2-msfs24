"""Exporta o greybox exterior (gerado por generate_greybox.py) como glTF.

Ref: MSFS 2024 SDK - docs.flightsimulator.com,
"5_Content_Configuration/Models/Model_Files_Setup.htm" (exemplo do
tutorial oficial "tutorial_aircraft"): o exterior de uma aeronave é um
arquivo .gltf referenciado por um XML de LOD (ver
PackageSources/.../model/embraer_e195e2_greybox.xml), que por sua vez é
referenciado pelo model.cfg. Este script produz só a parte .gltf/.bin.

Exporta APENAS os objetos "GREYBOX_*" (fuselagem/asas/empenagem/naceles)
- os marcadores CONTACT_* (trem de pouso, usados só como referência
visual para preencher [CONTACT_POINTS] em flight_model.cfg, não fazem
parte de um SimObject 3D real) e as telas SCREEN_DU* (interior/cockpit -
ainda não tem modelo de interior real nem o binding de glasscockpit
resolvido, ver tools/blender/README.md) ficam de fora deste export
exterior de proposito, para não ficar um modelo com telas soltas no meio
do nada.

Rodar headless:
    blender --background --python tools/blender/export_greybox_gltf.py
"""

import sys

import bpy

sys.path.insert(0, r"C:\Dev\Games\embraer-195-e2\tools\blender")
import generate_greybox  # noqa: E402

OUTPUT_PATH = (
    r"C:\Dev\Games\embraer-195-e2\PackageSources\SimObjects\Airplanes"
    r"\embraer-e195-e2\model\embraer_e195e2_greybox.gltf"
)


def main() -> None:
    generate_greybox.main()

    if "io_scene_gltf2" not in bpy.context.preferences.addons:
        bpy.ops.preferences.addon_enable(module="io_scene_gltf2")

    bpy.ops.object.select_all(action="DESELECT")
    exported = []
    for obj in bpy.data.objects:
        if obj.name.startswith("GREYBOX_"):
            obj.select_set(True)
            exported.append(obj.name)

    bpy.ops.export_scene.gltf(
        filepath=OUTPUT_PATH,
        export_format="GLTF_SEPARATE",
        use_selection=True,
        export_yup=True,
    )

    print(f"[export_greybox_gltf] exportado ({len(exported)} objetos): {OUTPUT_PATH}")
    for name in exported:
        print(f"  - {name}")


if __name__ == "__main__":
    main()
