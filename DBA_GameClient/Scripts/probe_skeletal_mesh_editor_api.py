# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeSkelMeshAPI] {msg}")


def dump_type(label: str, cls) -> None:
    if not cls:
        log(f"{label}=<none>")
        return
    methods = [name for name in dir(cls) if "skeleton" in name.lower() or "mesh" in name.lower()]
    methods = sorted(set(methods))
    log(f"{label} methods={','.join(methods)}")


def main() -> None:
    dump_type("SkeletalMeshEditorSubsystem", unreal.SkeletalMeshEditorSubsystem)
    dump_type("SkeletalMeshEditingLibrary", getattr(unreal, "SkeletalMeshEditingLibrary", None))
    dump_type("EditorSkeletalMeshLibrary", getattr(unreal, "EditorSkeletalMeshLibrary", None))

    try:
        subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
        methods = [name for name in dir(subsystem) if "skeleton" in name.lower() or "mesh" in name.lower()]
        methods = sorted(set(methods))
        log(f"subsystem_instance methods={','.join(methods)}")
    except Exception as exc:
        log(f"subsystem_instance error={exc}")


if __name__ == "__main__":
    main()
