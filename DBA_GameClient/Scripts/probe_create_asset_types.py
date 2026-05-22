# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal

root = "/Game/_AutoPlaceholdersTemp"
tools = unreal.AssetToolsHelpers.get_asset_tools()

def ensure_path(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

ensure_path(root)

results = []

def try_create(name, cls, factory):
    try:
        asset = tools.create_asset(name, root, cls, factory)
        results.append((name, bool(asset), str(asset.get_class().get_name()) if asset else "None"))
    except Exception as e:
        results.append((name, False, f"ERR:{e}"))

try:
    try_create("P_Test", unreal.ParticleSystem, unreal.ParticleSystemFactoryNew())
except Exception as e:
    results.append(("P_Test_ctor", False, f"ERR:{e}"))

try:
    try_create("SK_Test", unreal.SkeletalMesh, unreal.SkeletalMeshFactory())
except Exception as e:
    results.append(("SK_Test_ctor", False, f"ERR:{e}"))

try:
    try_create("ABP_Test", unreal.AnimBlueprint, unreal.AnimBlueprintFactory())
except Exception as e:
    results.append(("ABP_Test_ctor", False, f"ERR:{e}"))

try:
    try_create("AM_Test", unreal.AnimMontage, unreal.AnimMontageFactory())
except Exception as e:
    results.append(("AM_Test_ctor", False, f"ERR:{e}"))

for item in results:
    unreal.log_warning(f"[ProbeCreate] {item[0]} ok={item[1]} detail={item[2]}")

unreal.EditorAssetLibrary.save_directory(root, False, True)
