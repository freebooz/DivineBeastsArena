# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import runpy
import traceback
import unreal


def log(msg: str) -> None:
    unreal.log(f"[RunStarterRetargetAndABP] {msg}")


def run_script(path: str) -> None:
    log(f"running: {path}")
    runpy.run_path(path, run_name="__main__")
    log(f"completed: {path}")


def main() -> None:
    scripts = [
        r"D:/DivineBeastsArena/Scripts/apply_starter_skeleton_and_retarget.py",
        r"D:/DivineBeastsArena/Scripts/generate_animation_blueprint_scripts.py",
    ]
    for path in scripts:
        try:
            run_script(path)
        except Exception:
            log(f"failed: {path}")
            traceback.print_exc()
            raise
    log("all done")
    try:
        unreal.SystemLibrary.quit_editor()
    except Exception:
        pass


if __name__ == "__main__":
    main()
