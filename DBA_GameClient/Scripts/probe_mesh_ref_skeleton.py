# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeMeshRefSkeleton] {msg}")


def inspect_mesh(path: str) -> None:
    mesh = unreal.load_asset(path)
    if not isinstance(mesh, unreal.SkeletalMesh):
        log(f"mesh missing: {path}")
        return
    log(f"mesh={mesh.get_path_name()}")
    for prop in ["skeleton", "ref_skeleton"]:
        try:
            value = mesh.get_editor_property(prop)
            log(f"prop {prop} type={type(value)} value={value}")
            names = [n for n in dir(value) if "bone" in n.lower() or "num" in n.lower() or "name" in n.lower()]
            log(f"prop {prop} methods={','.join(sorted(set(names)))}")
        except Exception as ex:
            log(f"prop {prop} error={ex}")


def main() -> None:
    inspect_mesh("/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin")
    inspect_mesh("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat")


if __name__ == "__main__":
    main()
