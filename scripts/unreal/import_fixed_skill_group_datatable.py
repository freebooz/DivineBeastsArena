"""
Import DT_FixedSkillGroups.csv into a Unreal DataTable asset.

This script is intended to be launched by scripts/import-fixed-skill-group-datatable.ps1.
It writes only the requested /Game/DBA/Data/Tables/DT_FixedSkillGroups asset.
"""

import csv
import os
import sys

import unreal


EXPECTED_ROW_COUNT = 60
DEFAULT_ASSET_PACKAGE_PATH = "/Game/DBA/Data/Tables/DT_FixedSkillGroups"
DEFAULT_ROW_STRUCT_PATH = "/Script/DivineBeastsArena.DBAZodiacElementFixedSkillGroupRow"


def _get_arg(name: str, default: str = "") -> str:
    prefix = f"-{name}="
    for arg in sys.argv:
        if arg.startswith(prefix):
            return arg[len(prefix):]
    return default


def _get_arg_or_env(name: str, env_name: str, default: str = "") -> str:
    value = _get_arg(name, "")
    if value:
        return value
    return os.environ.get(env_name, default)


def _fail(message: str) -> None:
    unreal.log_error(f"[FixedSkillGroupImport] {message}")
    raise RuntimeError(message)


def _validate_csv(csv_path: str) -> None:
    if not os.path.isfile(csv_path):
        _fail(f"源 CSV 不存在：{csv_path}")

    with open(csv_path, "r", encoding="utf-8-sig", newline="") as handle:
        rows = list(csv.DictReader(handle))

    if len(rows) != EXPECTED_ROW_COUNT:
        _fail(f"源 CSV 行数不正确：期望 {EXPECTED_ROW_COUNT} 行，实际 {len(rows)} 行。")

    seen = set()
    for row in rows:
        row_name = row.get("Name", "")
        row_id = row.get("RowId", "")
        zodiac = row.get("ZodiacType", "")
        element = row.get("ElementType", "")
        expected = f"{zodiac}_{element}"

        if row_name != expected or row_id != expected:
            _fail(f"源 CSV 行身份不匹配：行={row_name}，期望={expected}。")

        if row.get("ResonanceElement", "") != element:
            _fail(f"源 CSV 共鸣元素不匹配：行={row_name}。")

        if row_name in seen:
            _fail(f"源 CSV 行重复：{row_name}")

        seen.add(row_name)


def _delete_existing_asset(asset_package_path: str) -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(asset_package_path):
        if not unreal.EditorAssetLibrary.delete_asset(asset_package_path):
            _fail(f"替换现有资产失败：{asset_package_path}")


def _import_data_table(csv_path: str, asset_package_path: str, row_struct_path: str) -> None:
    if not asset_package_path.startswith("/Game/DBA/Data/Tables/DT_FixedSkillGroups"):
        _fail(f"拒绝写入固定技能组 DataTable 之外的资产：{asset_package_path}")

    row_struct = unreal.load_object(None, row_struct_path)
    if not row_struct:
        _fail(f"行结构不存在：{row_struct_path}")

    asset_path, asset_name = asset_package_path.rsplit("/", 1)
    factory = unreal.CSVImportFactory()
    factory.automated_import_settings.import_row_struct = row_struct

    task = unreal.AssetImportTask()
    task.filename = csv_path
    task.destination_path = asset_path
    task.destination_name = asset_name
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.factory = factory

    _delete_existing_asset(asset_package_path)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    if task.imported_object_paths is None or len(task.imported_object_paths) == 0:
        _fail("Unreal 资产导入没有返回已导入对象路径。")

    if not unreal.EditorAssetLibrary.does_asset_exist(asset_package_path):
        _fail(f"导入后的 DataTable 资产不存在：{asset_package_path}")

    asset = unreal.EditorAssetLibrary.load_asset(asset_package_path)
    if not isinstance(asset, unreal.DataTable):
        _fail(f"导入后的资产不是 DataTable：{asset_package_path}")

    row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(asset)
    if len(row_names) != EXPECTED_ROW_COUNT:
        _fail(f"导入后的 DataTable 行数不正确：期望 {EXPECTED_ROW_COUNT} 行，实际 {len(row_names)} 行。")

    if not unreal.EditorAssetLibrary.save_asset(asset_package_path, only_if_is_dirty=False):
        _fail(f"保存导入后的 DataTable 资产失败：{asset_package_path}")

    unreal.log(f"[FixedSkillGroupImport] 已导入 {len(row_names)} 行到 {asset_package_path}")


def main() -> None:
    csv_path = _get_arg_or_env("CsvPath", "DBA_FIXED_SKILL_GROUP_CSV")
    asset_package_path = _get_arg_or_env("AssetPackagePath", "DBA_FIXED_SKILL_GROUP_ASSET", DEFAULT_ASSET_PACKAGE_PATH)
    row_struct_path = _get_arg_or_env("RowStructPath", "DBA_FIXED_SKILL_GROUP_ROW_STRUCT", DEFAULT_ROW_STRUCT_PATH)

    _validate_csv(csv_path)
    _import_data_table(csv_path, asset_package_path, row_struct_path)


main()
