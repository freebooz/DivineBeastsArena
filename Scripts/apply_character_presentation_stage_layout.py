import unreal

MAP_PATH = "/Game/Maps/Lobby/LobbyMap"
BP_CLASS_PATH = "/Game/Blueprints/UI/Lobby/BP_DBA_CharacterPresentationActor.BP_DBA_CharacterPresentationActor_C"
FALLBACK_BP_CLASS_PATH = "/Game/DBA/UI/Lobby/Character/BP_DBA_CharacterPresentationActor.BP_DBA_CharacterPresentationActor_C"
NATIVE_CLASS_PATH = "/Script/DivineBeastsArena.DBACharacterPresentationActor"

TARGET_LABEL = "DBA_CharacterPresentationStage"
TARGET_LOCATION = unreal.Vector(0.0, 0.0, 0.0)
TARGET_ROTATION = unreal.Rotator(0.0, 0.0, 0.0)
TARGET_SCALE = unreal.Vector(1.0, 1.0, 1.0)


def log(msg: str) -> None:
    unreal.log(f"[ApplyPresentationStageLayout] {msg}")


def load_spawn_class():
    for class_path in (BP_CLASS_PATH, FALLBACK_BP_CLASS_PATH, NATIVE_CLASS_PATH):
        cls = unreal.load_class(None, class_path)
        if cls:
            log(f"Using class: {class_path}")
            return cls
    raise RuntimeError("Failed to load presentation actor class (BP and native both missing).")


def is_presentation_actor(actor: unreal.Actor) -> bool:
    cls_name = actor.get_class().get_name()
    return "DBACharacterPresentationActor" in cls_name or "BP_DBA_CharacterPresentationActor" in cls_name


def ensure_single_stage_actor(spawn_class):
    actors = unreal.EditorActorSubsystem().get_all_level_actors()
    stage_actors = [a for a in actors if is_presentation_actor(a)]

    if stage_actors:
        primary = stage_actors[0]
        for dup in stage_actors[1:]:
            unreal.EditorActorSubsystem().destroy_actor(dup)
            log(f"Destroyed duplicate actor: {dup.get_name()}")
        actor = primary
        log(f"Reusing actor: {actor.get_name()}")
    else:
        actor = unreal.EditorActorSubsystem().spawn_actor_from_class(spawn_class, TARGET_LOCATION, TARGET_ROTATION)
        if not actor:
            raise RuntimeError("Failed to spawn presentation actor.")
        log(f"Spawned new actor: {actor.get_name()}")

    actor.set_actor_label(TARGET_LABEL)
    actor.set_actor_location(TARGET_LOCATION, False, True)
    actor.set_actor_rotation(TARGET_ROTATION, False)
    actor.set_actor_scale3d(TARGET_SCALE)
    log("Applied stage transform/location.")


def main():
    if not unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH):
        raise RuntimeError(f"Failed to load map: {MAP_PATH}")
    log(f"Loaded map: {MAP_PATH}")

    spawn_class = load_spawn_class()
    ensure_single_stage_actor(spawn_class)

    if not unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True):
        raise RuntimeError("Failed to save dirty packages after stage layout apply.")
    log("Saved map/packages.")


if __name__ == "__main__":
    main()
