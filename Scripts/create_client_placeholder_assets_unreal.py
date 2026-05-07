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
    paths = gather_paths()
    log(f"scanned_paths={len(paths)}")

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
