# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import inspect
import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeRetargetSig] {msg}")


def main() -> None:
    fn = unreal.IKRetargetBatchOperation.duplicate_and_retarget
    try:
        sig = inspect.signature(fn)
        log(f"signature={sig}")
    except Exception as exc:
        log(f"signature_error={exc}")

    doc = getattr(fn, "__doc__", None)
    if doc:
        log(f"doc={doc}")


if __name__ == "__main__":
    main()
