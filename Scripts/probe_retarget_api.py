import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeRetargetAPI] {msg}")


def dump(label: str, obj) -> None:
    if not obj:
        log(f"{label}=<none>")
        return
    names = sorted(set(dir(obj)))
    filtered = [n for n in names if any(k in n.lower() for k in ["retarget", "ik", "rig", "anim", "batch"])]
    log(f"{label}={','.join(filtered)}")


def main() -> None:
    dump("IKRetargetBatchOperation", getattr(unreal, "IKRetargetBatchOperation", None))
    dump("IKRetargeter", getattr(unreal, "IKRetargeter", None))
    dump("IKRetargetFactory", getattr(unreal, "IKRetargetFactory", None))
    dump("IKRigDefinition", getattr(unreal, "IKRigDefinition", None))
    dump("IKRigController", getattr(unreal, "IKRigController", None))
    dump("AnimationBlueprintLibrary", getattr(unreal, "AnimationBlueprintLibrary", None))
    dump("EditorAssetLibrary", unreal.EditorAssetLibrary)
    dump("AssetTools", unreal.AssetToolsHelpers.get_asset_tools())


if __name__ == "__main__":
    main()
