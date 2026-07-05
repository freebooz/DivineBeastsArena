# Frostbolt VFX Design Spec

**Date:** 2026-06-26

**Goal**

Create an original blue-white frost mage projectile package under `/Game/FrostMage` that is self-contained, testable in PIE, and does not modify existing core character, `PlayerController`, or `GameInstance` assets.

**Design Constraints**

- All new assets live under `/Game/FrostMage`.
- No direct editing of `.uasset` or `.umap` binaries.
- Use `Monolith` for project analysis.
- Use `VibeUE` or Unreal MCP for editor-side asset creation, compilation, save, and PIE.
- Use `UnrealMotionGraphicsMCP` for Widget creation and JSON export.
- Create assets in small batches and save immediately after each batch.
- If a required tool is unavailable, stop and report instead of guessing.

**Read-Only Findings**

- Existing projectile flow is centered on `ADBASkillProjectileBase`, which already provides collision, `ProjectileMovement`, Niagara presentation hooks, async preload, and impact feedback.
- Existing frost projectile presentation in `ADBAFrostShardProjectile` uses dynamic material instances plus layered Niagara wake and impact effects.
- Existing arena UI includes `UDBASelfCastBarWidgetBase`, which is a safe parent class for a test-only cast bar without touching production HUD assets.
- `UmgMcp` exposes `create_widget`, `set_widget_properties`, `save_asset`, and `export_umg_to_json`.

**Proposed Architecture**

- `BP_FrostboltProjectile` will be a self-contained projectile blueprint that reuses the existing projectile base behavior and attaches new FrostMage materials and Niagara systems.
- `BP_FrostMageCaster` will be a dedicated test actor placed in a test map. It will own minimal test input logic for spawning Frostbolt and driving the debug UI.
- `BP_Frostbolt_TestTarget` will be a simple hit target actor used to validate impact, collision, and debug output.
- `WBP_FrostboltCastBar` and `WBP_FrostboltDebugHUD` will be stand-alone test widgets, created via `UmgMcp` and exported to JSON after creation.
- `L_Frostbolt_Test` will isolate all validation to a dedicated test map.

**Asset List**

- Materials
  - `/Game/FrostMage/Materials/M_Frost_Core`
  - `/Game/FrostMage/Materials/M_Frost_Ribbon`
  - `/Game/FrostMage/Materials/M_Frost_Shard`
  - `/Game/FrostMage/Materials/M_Frost_Mist`
  - `/Game/FrostMage/Materials/M_Frost_Decal`
- Niagara
  - `/Game/FrostMage/VFX/NS_Frostbolt`
  - `/Game/FrostMage/VFX/NS_Frostbolt_Impact`
  - `/Game/FrostMage/VFX/NS_Frostbolt_Cast`
- Blueprints
  - `/Game/FrostMage/Blueprints/BP_FrostboltProjectile`
  - `/Game/FrostMage/Blueprints/BP_FrostMageCaster`
  - `/Game/FrostMage/Blueprints/BP_Frostbolt_TestTarget`
- UI
  - `/Game/FrostMage/UI/WBP_FrostboltCastBar`
  - `/Game/FrostMage/UI/WBP_FrostboltDebugHUD`
- Map
  - `/Game/FrostMage/Levels/L_Frostbolt_Test`

**Visual Direction**

- Core palette: ice white, cyan-blue, pale desaturated blue.
- Shape language: needle-like shard core with soft mist envelope, not a fireball silhouette.
- Motion: fast linear missile, slight ribbon twist, crisp crystal burst on impact.
- Surface response: translucent emissive core, metallic glints on shard layers, cold decal on ground hit.

**Validation Requirements**

- Compile all created Blueprints and Widgets.
- Save all created assets.
- Run PIE smoke test in `L_Frostbolt_Test`.
- Inspect Output Log for compile/runtime errors.
- Report added assets, verification results, and Git diff summary.
