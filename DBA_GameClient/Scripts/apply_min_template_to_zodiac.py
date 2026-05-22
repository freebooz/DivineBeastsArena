# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import re
from pathlib import Path

import unreal


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SCAN_DIRS = [
    PROJECT_ROOT / "Source/DivineBeastsArena/Private/GameDBA/Character/Zodiac",
    PROJECT_ROOT / "Source/DivineBeastsArena/Private/GameDBA/Animation/Zodiac",
]

TEMPLATE_SK = "/Game/Templates/MinimalCharacter/SK_MinTemplate.SK_MinTemplate"
TEMPLATE_ABP = "/Game/Templates/MinimalCharacter/ABP_MinTemplate.ABP_MinTemplate"
TEMPLATE_AM = "/Game/Templates/MinimalCharacter/AM_MinTemplate.AM_MinTemplate"


def log(msg: str) -> None:
    unreal.log(f"[ApplyMinTemplate] {msg}")


def gather_paths() -> list[str]:
    pattern = re.compile(r'TEXT\("(/Game/[^"]+)"\)')
    results = set()
    for scan_dir in SCAN_DIRS:
        if not scan_dir.exists():
            continue
        for cpp_file in scan_dir.rglob("*.cpp"):
            content = cpp_file.read_text(encoding="utf-8", errors="ignore")
            for match in pattern.findall(content):
                if (
                    "/Game/Models/Zodiac/" in match
                    or "/Game/Animation/Zodiac/" in match
                ):
                    results.add(match)
    return sorted(results)


def infer_kind(object_path: str) -> str:
    if "/Game/Models/" in object_path:
        return "SkeletalMesh"
    if "/Montages/" in object_path:
        return "AnimMontage"
    if "/ABP_" in object_path:
        return "AnimBlueprint"
    return "Unknown"


def ensure_from_template(object_path: str, template_path: str) -> bool:
    lib = unreal.EditorAssetLibrary
    template_obj = lib.load_asset(template_path)
    if not template_obj:
        log(f"template load failed: {template_path}")
        return False

    if lib.does_asset_exist(object_path):
        return True

    package_path, asset_name = object_path.rsplit(".", 1)[0].rsplit("/", 1)
    lib.make_directory(package_path)
    duplicated = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset(
        asset_name=asset_name,
        package_path=package_path,
        original_object=template_obj,
    )
    if not duplicated:
        return False
    unreal.EditorAssetLibrary.save_loaded_asset(duplicated)
    return True


def main() -> None:
    lib = unreal.EditorAssetLibrary
    for required in [TEMPLATE_SK, TEMPLATE_ABP, TEMPLATE_AM]:
        if not lib.does_asset_exist(required):
            raise RuntimeError(f"missing template asset: {required}")

    paths = gather_paths()
    created = 0
    failed = 0
    skipped = 0

    for p in paths:
        kind = infer_kind(p)
        if kind == "SkeletalMesh":
            ok = ensure_from_template(p, TEMPLATE_SK)
        elif kind == "AnimBlueprint":
            ok = ensure_from_template(p, TEMPLATE_ABP)
        elif kind == "AnimMontage":
            ok = ensure_from_template(p, TEMPLATE_AM)
        else:
            skipped += 1
            continue

        if ok:
            created += 1
            log(f"applied {kind} -> {p}")
        else:
            failed += 1
            log(f"failed {kind} -> {p}")

    log(f"summary total={len(paths)} created={created} failed={failed} skipped={skipped}")


if __name__ == "__main__":
    main()
