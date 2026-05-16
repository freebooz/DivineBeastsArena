import unreal


TARGET_ROOT = "/Game/DBA/Heroes"
OUTPUT_ROOT = "/Game/DBA/Heroes/Animation"
PARENT_ANIM_CLASS_PATH = "/Script/DivineBeastsArena.DBAZodiacAnimInstance"

# If your imported mesh is not under TARGET_ROOT yet, add explicit mesh paths here.
FORCE_MESHES: list[str] = []

FALLBACK_MESHES: list[str] = []

# These templates already contain AnimGraph/StateMachine/OutputPose.
TEMPLATE_ANIM_BLUEPRINTS = [
    "/Game/DBA/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed",
    "/Game/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny",
    "/Engine/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny",
    "/Game/DBA/Characters/Rosales/AnimationBP/ABP_Rosales.ABP_Rosales",
    "/Game/DBA/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny",
]


def log(msg: str) -> None:
    unreal.log(f"[GenerateHeroAnimBP] {msg}")


def load_asset(asset_path: str):
    return unreal.load_asset(asset_path)


def list_assets_by_class(root: str, class_name: str) -> list[str]:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    filt = unreal.ARFilter(
        class_paths=[unreal.TopLevelAssetPath(f"/Script/Engine.{class_name}")],
        package_paths=[unreal.Name(root)],
        recursive_paths=True,
        recursive_classes=True,
    )
    assets = registry.get_assets(filt)
    return [f"{a.package_name}.{a.asset_name}" for a in assets]


def collect_target_meshes() -> list[unreal.SkeletalMesh]:
    meshes: list[unreal.SkeletalMesh] = []
    seen = set()

    for object_path in list_assets_by_class(TARGET_ROOT, "SkeletalMesh"):
        mesh = load_asset(object_path)
        if mesh and mesh.get_path_name() not in seen:
            meshes.append(mesh)
            seen.add(mesh.get_path_name())

    if not meshes:
        try:
            selected_assets = unreal.EditorUtilityLibrary.get_selected_assets()
        except Exception:
            selected_assets = []
        for obj in selected_assets:
            if isinstance(obj, unreal.SkeletalMesh) and obj.get_path_name() not in seen:
                meshes.append(obj)
                seen.add(obj.get_path_name())

    for object_path in FORCE_MESHES:
        mesh = load_asset(object_path)
        if isinstance(mesh, unreal.SkeletalMesh) and mesh.get_path_name() not in seen:
            meshes.append(mesh)
            seen.add(mesh.get_path_name())

    if not meshes:
        for object_path in FALLBACK_MESHES:
            mesh = load_asset(object_path)
            if isinstance(mesh, unreal.SkeletalMesh) and mesh.get_path_name() not in seen:
                meshes.append(mesh)
                seen.add(mesh.get_path_name())
        if meshes:
            log("using fallback meshes because /Game/DBA/Heroes has no SkeletalMesh yet.")

    return meshes


def get_template_infos() -> list[tuple[unreal.AnimBlueprint, unreal.Skeleton]]:
    infos = []
    for object_path in TEMPLATE_ANIM_BLUEPRINTS:
        abp = load_asset(object_path)
        if not isinstance(abp, unreal.AnimBlueprint):
            continue
        try:
            skeleton = abp.get_editor_property("target_skeleton")
        except Exception:
            skeleton = None
        if skeleton:
            infos.append((abp, skeleton))
    return infos


def strip_mesh_prefix(mesh_name: str) -> str:
    prefixes = ["SK_", "SKM_", "SkeletalMesh_", "SM_"]
    for prefix in prefixes:
        if mesh_name.startswith(prefix):
            return mesh_name[len(prefix):]
    return mesh_name


def resolve_output_name(mesh: unreal.SkeletalMesh) -> str:
    return f"ABP_{strip_mesh_prefix(mesh.get_name())}"


def ensure_output_directory() -> None:
    unreal.EditorAssetLibrary.make_directory(OUTPUT_ROOT)


def create_abp_from_template(
    output_name: str, template_abp: unreal.AnimBlueprint
) -> unreal.AnimBlueprint | None:
    object_path = f"{OUTPUT_ROOT}/{output_name}.{output_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(object_path):
        existing = load_asset(object_path)
        return existing if isinstance(existing, unreal.AnimBlueprint) else None

    duplicated = unreal.AssetToolsHelpers.get_asset_tools().duplicate_asset(
        asset_name=output_name,
        package_path=OUTPUT_ROOT,
        original_object=template_abp,
    )
    if isinstance(duplicated, unreal.AnimBlueprint):
        unreal.EditorAssetLibrary.save_loaded_asset(duplicated)
        return duplicated
    return None


def create_blank_abp(output_name: str, skeleton: unreal.Skeleton) -> unreal.AnimBlueprint | None:
    object_path = f"{OUTPUT_ROOT}/{output_name}.{output_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(object_path):
        existing = load_asset(object_path)
        return existing if isinstance(existing, unreal.AnimBlueprint) else None

    factory = unreal.AnimBlueprintFactory()
    factory.set_editor_property("target_skeleton", skeleton)

    parent_class = unreal.load_class(None, PARENT_ANIM_CLASS_PATH)
    if parent_class:
        try:
            factory.set_editor_property("parent_class", parent_class)
        except Exception:
            log("warning: AnimBlueprintFactory.parent_class is unavailable in this engine build.")

    created = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        output_name, OUTPUT_ROOT, unreal.AnimBlueprint, factory
    )
    if isinstance(created, unreal.AnimBlueprint):
        unreal.EditorAssetLibrary.save_loaded_asset(created)
        return created
    return None


def suggest_state_assets_for_mesh(mesh: unreal.SkeletalMesh) -> dict[str, str]:
    suggestions: dict[str, str] = {}
    mesh_folder = mesh.get_path_name().split(".")[0].rsplit("/", 1)[0]
    candidate_roots = [
        mesh_folder,
        mesh_folder.replace("/Meshes", "/Animations"),
        mesh_folder.replace("/Meshes", "/Animation"),
        mesh_folder.replace("/Visuals/Meshes", "/Animations"),
    ]

    all_sequences: list[str] = []
    for root in candidate_roots:
        for seq_path in list_assets_by_class(root, "AnimSequence"):
            all_sequences.append(seq_path)

    unique_sequences = []
    seen = set()
    for seq_path in all_sequences:
        if seq_path not in seen:
            unique_sequences.append(seq_path)
            seen.add(seq_path)

    keywords = {
        "Idle": ["idle", "stand"],
        "Walk": ["walk"],
        "Run": ["run", "jog", "sprint"],
        "Jump": ["jump", "jump_up"],
        "Fall": ["fall", "fall_loop"],
        "Attack": ["attack", "atk"],
        "Hit": ["hit", "hurt", "impact"],
        "Death": ["death", "die", "dead"],
    }

    for state, words in keywords.items():
        for seq_path in unique_sequences:
            lower = seq_path.lower()
            if any(word in lower for word in words):
                suggestions[state] = seq_path
                break

    return suggestions


def main() -> None:
    ensure_output_directory()

    meshes = collect_target_meshes()
    if not meshes:
        log(
            "no target skeletal meshes found. "
            "Import mesh to /Game/DBA/Heroes, or select mesh in Content Browser, or fill FORCE_MESHES."
        )
        return

    template_infos = get_template_infos()
    created_count = 0
    reused_count = 0

    for mesh in meshes:
        if not mesh:
            continue

        skeleton = mesh.get_editor_property("skeleton")
        if not skeleton:
            log(f"skip mesh={mesh.get_path_name()} reason=no_skeleton")
            continue

        output_name = resolve_output_name(mesh)
        output_object_path = f"{OUTPUT_ROOT}/{output_name}.{output_name}"
        existed_before = unreal.EditorAssetLibrary.does_asset_exist(output_object_path)

        created_abp = None
        created_mode = "blank"
        template_used = None

        for template_abp, template_skeleton in template_infos:
            if template_skeleton == skeleton:
                template_used = template_abp
                break

        if template_used:
            created_abp = create_abp_from_template(output_name, template_used)
            created_mode = "template"
        else:
            created_abp = create_blank_abp(output_name, skeleton)

        if not created_abp:
            log(f"failed create mesh={mesh.get_path_name()} output={output_object_path}")
            continue

        if existed_before:
            reused_count += 1
        else:
            created_count += 1

        unreal.EditorAssetLibrary.save_loaded_asset(created_abp)
        log(
            "created "
            f"mesh={mesh.get_path_name()} skeleton={skeleton.get_path_name()} "
            f"abp={created_abp.get_path_name()} mode={created_mode} "
            f"template={template_used.get_path_name() if template_used else '<none>'}"
        )

        suggestions = suggest_state_assets_for_mesh(mesh)
        if suggestions:
            for state_name, seq_path in suggestions.items():
                log(f"suggest {output_name} state={state_name} asset={seq_path}")
        else:
            log(
                f"suggest {output_name} no local AnimSequence matches found. "
                "You can assign Idle/Walk/Run/Jump/Fall/Attack/Hit/Death manually."
            )

        if created_mode != "template":
            log(
                f"note {output_name}: this ABP is scaffolded with parent class only. "
                "To get full StateMachine+OutputPose automatically, provide a template ABP using the same skeleton."
            )

    log(f"summary meshes={len(meshes)} created_or_reused={created_count} reused_existing={reused_count}")


if __name__ == "__main__":
    main()
