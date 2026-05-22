# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeSetSkeleton] {msg}")


def main() -> None:
    candidates = [
        ("EditorSkeletalMeshLibrary", getattr(unreal, "EditorSkeletalMeshLibrary", None)),
        ("SkeletalMeshEditorSubsystem", getattr(unreal, "SkeletalMeshEditorSubsystem", None)),
        ("IKRetargetBatchOperation", getattr(unreal, "IKRetargetBatchOperation", None)),
    ]
    method_names = [
        "set_skeleton",
        "assign_skeleton",
        "set_skeletal_mesh_skeleton",
        "retarget_skeleton",
        "replace_skeleton",
    ]
    for label, obj in candidates:
        if not obj:
            log(f"{label}=<none>")
            continue
        for name in method_names:
            has = hasattr(obj, name)
            log(f"{label}.{name}={has}")

    mesh = unreal.load_asset("/Game/DBA/Characters/Rosales/Meshes/SK_Rosales.SK_Rosales")
    skel = unreal.load_asset("/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin")
    if mesh and skel:
        log(f"mesh_has_set_skeleton={hasattr(mesh, 'set_skeleton')}")
        if hasattr(mesh, "set_skeleton"):
            try:
                mesh.set_skeleton(skel)
                log("mesh.set_skeleton call succeeded")
            except Exception as exc:
                log(f"mesh.set_skeleton call failed: {exc}")

        for method in [
            ("EditorSkeletalMeshLibrary.set_skeleton", getattr(getattr(unreal, "EditorSkeletalMeshLibrary", object), "set_skeleton", None)),
            ("SkeletalMeshEditorSubsystem.set_skeleton", getattr(unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem), "set_skeleton", None)),
        ]:
            name, fn = method
            if not fn:
                continue
            try:
                fn(mesh, skel)
                log(f"{name} call succeeded")
            except Exception as exc:
                log(f"{name} call failed: {exc}")


if __name__ == "__main__":
    main()
