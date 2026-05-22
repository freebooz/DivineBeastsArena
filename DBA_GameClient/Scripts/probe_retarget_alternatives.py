# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeRetargetAlt] {msg}")


def dump_obj(name: str, obj) -> None:
    if not obj:
        log(f"{name}=<none>")
        return
    names = sorted(set(dir(obj)))
    filtered = [n for n in names if any(k in n.lower() for k in ["retarget", "ik", "rig", "anim", "batch", "duplicate"])]
    log(f"{name} methods={','.join(filtered)}")


def main() -> None:
    for attr in sorted(dir(unreal)):
        lower = attr.lower()
        if "retarget" in lower or "ikrig" in lower:
            try:
                obj = getattr(unreal, attr)
                dump_obj(attr, obj)
            except Exception as ex:
                log(f"{attr} error={ex}")

    try:
        subsystem = unreal.get_editor_subsystem(unreal.IKRigEditorSubsystem)
        dump_obj("IKRigEditorSubsystemInstance", subsystem)
    except Exception as ex:
        log(f"IKRigEditorSubsystem unavailable: {ex}")


if __name__ == "__main__":
    main()
