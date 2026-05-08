import re
from pathlib import Path

import unreal


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SCAN_DIRS = [
    PROJECT_ROOT / "Source/DivineBeastsArena/Private/GameDBA/Character/Zodiac",
    PROJECT_ROOT / "Source/DivineBeastsArena/Private/GameDBA/Animation/Zodiac",
    PROJECT_ROOT / "Source/DivineBeastsArena/Private/GameDBA/Combat/Projectiles",
]


def log(msg: str) -> None:
    unreal.log(f"[ClientAssetBootstrap] {msg}")


def infer_kind(object_path: str) -> str:
    if "/Game/Models/" in object_path:
        return "SkeletalMesh"
    if "/Montages/" in object_path:
        return "AnimMontage"
    if "/ABP_" in object_path:
        return "AnimBlueprint"
    if "/Game/VFX/" in object_path:
        return "ParticleSystem"
    if "/Game/Audio/" in object_path:
        return "SoundBase"
    return "Unknown"


def gather_paths() -> list[str]:
    pattern = re.compile(r'TEXT\("(/Game/[^"]+)"\)')
    results = set()
    for scan_dir in SCAN_DIRS:
        if not scan_dir.exists():
            continue
        for cpp_file in scan_dir.rglob("*.cpp"):
            try:
                content = cpp_file.read_text(encoding="utf-8", errors="ignore")
            except Exception:
                continue
            for match in pattern.findall(content):
                # Only client resource domains requested by user.
                if (
                    "/Game/Models/" in match
                    or "/Game/Animation/" in match
                    or "/Game/VFX/" in match
                    or "/Game/Audio/" in match
                ):
                    results.add(match)
    return sorted(results)


def find_template_for_kind(kind: str, registry: unreal.AssetRegistry) -> str | None:
    class_candidates = {
        "SkeletalMesh": ["SkeletalMesh"],
        "AnimMontage": ["AnimMontage"],
        "AnimBlueprint": ["AnimBlueprint"],
        "ParticleSystem": ["ParticleSystem"],
        "SoundBase": ["SoundCue", "SoundWave"],
    }

    for cls_name in class_candidates.get(kind, []):
        cls_path = unreal.TopLevelAssetPath(f"/Script/Engine.{cls_name}")
        assets = registry.get_assets_by_class(cls_path, True)
        if assets:
            package_name = str(assets[0].package_name)
            asset_name = str(assets[0].asset_name)
            return f"{package_name}.{asset_name}"

    return None


def create_bootstrap_templates() -> dict[str, str]:
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    lib = unreal.EditorAssetLibrary
    created: dict[str, str] = {}
    root = "/Game/_AutoPlaceholders"
    lib.make_directory(root)

    def _ensure_asset(path: str, creator):
        if lib.does_asset_exist(path):
            return lib.load_asset(path)
        return creator()

    # Build a minimal animation chain from a known engine skeletal mesh template.
    sk_path = f"{root}/SK_Placeholder.SK_Placeholder"
    if lib.does_asset_exist(sk_path):
        sk_asset = lib.load_asset(sk_path)
    else:
        engine_sk = unreal.load_asset("/Engine/EngineMeshes/SkeletalCube.SkeletalCube")
        sk_asset = None
        if engine_sk:
            sk_asset = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset(
                asset_name="SK_Placeholder",
                package_path=root,
                original_object=engine_sk,
            )
    if sk_asset:
        created["SkeletalMesh"] = sk_path
        unreal.EditorAssetLibrary.save_loaded_asset(sk_asset)

        skeleton = None
        try:
            skeleton = sk_asset.get_editor_property("skeleton")
        except Exception:
            skeleton = None

        if skeleton:
            abp_path = f"{root}/ABP_Placeholder.ABP_Placeholder"
            if lib.does_asset_exist(abp_path):
                created["AnimBlueprint"] = abp_path
            else:
                try:
                    abp_factory = unreal.AnimBlueprintFactory()
                    try:
                        abp_factory.set_editor_property("target_skeleton", skeleton)
                    except Exception:
                        pass
                    abp = tools.create_asset("ABP_Placeholder", root, unreal.AnimBlueprint, abp_factory)
                    if abp:
                        created["AnimBlueprint"] = abp_path
                        unreal.EditorAssetLibrary.save_loaded_asset(abp)
                except Exception as e:
                    log(f"AnimBlueprint bootstrap failed: {e}")

            am_path = f"{root}/AM_Placeholder.AM_Placeholder"
            if lib.does_asset_exist(am_path):
                created["AnimMontage"] = am_path
            else:
                try:
                    am_factory = unreal.AnimMontageFactory()
                    am_factory.set_editor_property("target_skeleton", skeleton)
                    am = tools.create_asset("AM_Placeholder", root, unreal.AnimMontage, am_factory)
                    if am:
                        created["AnimMontage"] = am_path
                        unreal.EditorAssetLibrary.save_loaded_asset(am)
                except Exception as e:
                    log(f"AnimMontage bootstrap failed: {e}")

    particle_path = f"{root}/P_Placeholder.P_Placeholder"
    p = _ensure_asset(
        particle_path,
        lambda: tools.create_asset("P_Placeholder", root, unreal.ParticleSystem, unreal.ParticleSystemFactoryNew()),
    )
    if p:
        created["ParticleSystem"] = particle_path

    cue_path = f"{root}/S_Placeholder.S_Placeholder"
    if lib.does_asset_exist(cue_path):
        created["SoundBase"] = cue_path

    for pth in created.values():
        asset = lib.load_asset(pth)
        if asset:
            unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return created


def cleanup_bad_temp_assets() -> None:
    lib = unreal.EditorAssetLibrary
    temp_root = "/Game/_AutoPlaceholdersTemp"
    if lib.does_directory_exist(temp_root):
        assets = lib.list_assets(temp_root, True, True)
        for asset in assets:
            lib.delete_asset(asset)
        lib.delete_directory(temp_root)


def cleanup_generated_zodiac_placeholders(paths: list[str]) -> None:
    lib = unreal.EditorAssetLibrary
    for object_path in paths:
        kind = infer_kind(object_path)
        if kind not in {"SkeletalMesh", "AnimBlueprint", "AnimMontage"}:
            continue
        package_obj_path = object_path
        if not lib.does_asset_exist(package_obj_path):
            continue
        lib.delete_asset(package_obj_path)


def ensure_asset(object_path: str, template_path: str) -> tuple[bool, str]:
    lib = unreal.EditorAssetLibrary
    if lib.does_asset_exist(object_path):
        return True, "exists"

    package_path, asset_name = object_path.rsplit(".", 1)[0].rsplit("/", 1)
    lib.make_directory(package_path)

    original = lib.load_asset(template_path)
    if not original:
        return False, "template_load_failed"

    duplicated = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset(
        asset_name=asset_name,
        package_path=package_path,
        original_object=original,
    )
    if duplicated:
        unreal.EditorAssetLibrary.save_loaded_asset(duplicated)
        return True, f"created_from={template_path}"
    return False, "duplicate_failed"


def main() -> None:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    cleanup_bad_temp_assets()
    paths = gather_paths()
    cleanup_generated_zodiac_placeholders(paths)
    log(f"scanned_paths={len(paths)}")
    bootstrap_templates = create_bootstrap_templates()
    log(f"bootstrap_templates={bootstrap_templates}")

    created = 0
    exists = 0
    skipped = 0
    failed = 0

    templates_cache: dict[str, str | None] = {}

    for object_path in paths:
        kind = infer_kind(object_path)
        if kind == "Unknown":
            skipped += 1
            log(f"skip kind=Unknown path={object_path}")
            continue

        if kind not in templates_cache:
            templates_cache[kind] = find_template_for_kind(kind, registry)
            if not templates_cache[kind] and kind in bootstrap_templates:
                templates_cache[kind] = bootstrap_templates[kind]

        template = templates_cache[kind]
        if not template:
            skipped += 1
            log(f"skip no_template kind={kind} path={object_path}")
            continue

        ok, detail = ensure_asset(object_path, template)
        if ok and detail == "exists":
            exists += 1
        elif ok:
            created += 1
            log(f"created kind={kind} path={object_path} {detail}")
        else:
            failed += 1
            log(f"failed kind={kind} path={object_path} detail={detail}")

    log(
        f"summary created={created} exists={exists} skipped={skipped} failed={failed} total={len(paths)}"
    )


if __name__ == "__main__":
    main()
