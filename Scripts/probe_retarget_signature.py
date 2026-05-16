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
