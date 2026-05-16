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
