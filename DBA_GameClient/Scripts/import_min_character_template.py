# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


ROOT = "/Game/Templates/MinimalCharacter"
SK_NAME = "SK_MinTemplate"
ABP_NAME = "ABP_MinTemplate"
AM_NAME = "AM_MinTemplate"


def log(msg: str) -> None:
    unreal.log(f"[MinCharacterImport] {msg}")


def main() -> None:
    lib = unreal.EditorAssetLibrary
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    lib.make_directory(ROOT)

    src_sk = unreal.load_asset("/Engine/EngineMeshes/SkeletalCube.SkeletalCube")
    if not src_sk:
        raise RuntimeError("Cannot load source skeletal mesh: /Engine/EngineMeshes/SkeletalCube.SkeletalCube")

    sk_obj_path = f"{ROOT}/{SK_NAME}.{SK_NAME}"
    if lib.does_asset_exist(sk_obj_path):
        sk = lib.load_asset(sk_obj_path)
        log(f"exists {sk_obj_path}")
    else:
        sk = tools.duplicate_asset(SK_NAME, ROOT, src_sk)
        if not sk:
            raise RuntimeError("Failed to create SK_MinTemplate")
        log(f"created {sk_obj_path}")
    lib.save_loaded_asset(sk)

    skeleton = sk.get_editor_property("skeleton")
    if not skeleton:
        raise RuntimeError("SK_MinTemplate has no skeleton")

    abp_obj_path = f"{ROOT}/{ABP_NAME}.{ABP_NAME}"
    if not lib.does_asset_exist(abp_obj_path):
        abp_factory = unreal.AnimBlueprintFactory()
        try:
            abp_factory.set_editor_property("target_skeleton", skeleton)
        except Exception:
            pass
        abp = tools.create_asset(ABP_NAME, ROOT, unreal.AnimBlueprint, abp_factory)
        if not abp:
            raise RuntimeError("Failed to create ABP_MinTemplate")
        lib.save_loaded_asset(abp)
        log(f"created {abp_obj_path}")
    else:
        log(f"exists {abp_obj_path}")

    am_obj_path = f"{ROOT}/{AM_NAME}.{AM_NAME}"
    if not lib.does_asset_exist(am_obj_path):
        am_factory = unreal.AnimMontageFactory()
        am_factory.set_editor_property("target_skeleton", skeleton)
        am = tools.create_asset(AM_NAME, ROOT, unreal.AnimMontage, am_factory)
        if not am:
            raise RuntimeError("Failed to create AM_MinTemplate")
        lib.save_loaded_asset(am)
        log(f"created {am_obj_path}")
    else:
        log(f"exists {am_obj_path}")

    log("done")


if __name__ == "__main__":
    main()
