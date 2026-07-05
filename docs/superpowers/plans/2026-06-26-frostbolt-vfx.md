# Frostbolt VFX Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a self-contained Frostbolt VFX demo package under `/Game/FrostMage` with materials, Niagara, Blueprints, UMG, a test map, and end-to-end PIE validation.

**Architecture:** Reuse the existing projectile presentation pattern instead of changing core gameplay systems. Create a dedicated test caster, projectile, target, and widgets so the whole effect can be validated inside an isolated test map.

**Tech Stack:** Unreal Engine 5, Blueprint assets, Niagara, Materials, UMG via UmgMcp, editor automation via Unreal MCP and VibeUE, repository verification via git and local logs.

---

### Task 1: Create FrostMage Folder Structure and Base Materials

**Files:**
- Create: `/Game/FrostMage/Materials/M_Frost_Core`
- Create: `/Game/FrostMage/Materials/M_Frost_Ribbon`
- Create: `/Game/FrostMage/Materials/M_Frost_Shard`
- Create: `/Game/FrostMage/Materials/M_Frost_Mist`
- Create: `/Game/FrostMage/Materials/M_Frost_Decal`

- [ ] **Step 1: Create the five material assets under `/Game/FrostMage/Materials`**
- [ ] **Step 2: Configure shared parameter naming across all materials**
Parameter names:
`Tint`
`EmissiveStrength`
`Metallic`
`Opacity`
`FresnelPower`
- [ ] **Step 3: Compile each material after creation**
- [ ] **Step 4: Save material assets immediately after compile**

### Task 2: Create Frostbolt Niagara Systems

**Files:**
- Create: `/Game/FrostMage/VFX/NS_Frostbolt`
- Create: `/Game/FrostMage/VFX/NS_Frostbolt_Impact`
- Create: `/Game/FrostMage/VFX/NS_Frostbolt_Cast`

- [ ] **Step 1: Create `NS_Frostbolt` with core, shard, ribbon, and mist layers**
- [ ] **Step 2: Create `NS_Frostbolt_Impact` with crystal burst, ring burst, and fog layers**
- [ ] **Step 3: Create `NS_Frostbolt_Cast` with short pre-launch swirl and hand glow**
- [ ] **Step 4: Save each Niagara asset immediately after creation**

### Task 3: Create Projectile and Test Actors

**Files:**
- Create: `/Game/FrostMage/Blueprints/BP_FrostboltProjectile`
- Create: `/Game/FrostMage/Blueprints/BP_FrostMageCaster`
- Create: `/Game/FrostMage/Blueprints/BP_Frostbolt_TestTarget`

- [ ] **Step 1: Create `BP_FrostboltProjectile` from the safest existing projectile-compatible parent**
- [ ] **Step 2: Attach FrostMage materials and Niagara references to projectile presentation**
- [ ] **Step 3: Create `BP_FrostMageCaster` as a dedicated test actor that can spawn Frostbolt without touching core player systems**
- [ ] **Step 4: Create `BP_Frostbolt_TestTarget` as a simple hit-validation actor**
- [ ] **Step 5: Compile and save each Blueprint immediately after its edits**

### Task 4: Create UMG Widgets and Export JSON

**Files:**
- Create: `/Game/FrostMage/UI/WBP_FrostboltCastBar`
- Create: `/Game/FrostMage/UI/WBP_FrostboltDebugHUD`

- [ ] **Step 1: Create `WBP_FrostboltCastBar` with progress fill, spell label, and timing text using UmgMcp**
- [ ] **Step 2: Save the widget and export it to JSON**
- [ ] **Step 3: Create `WBP_FrostboltDebugHUD` with counters for projectile speed, lifetime, hit count, and last impact time using UmgMcp**
- [ ] **Step 4: Save the widget and export it to JSON**
- [ ] **Step 5: Compile both widgets**

### Task 5: Create Test Map and Place Actors

**Files:**
- Create: `/Game/FrostMage/Levels/L_Frostbolt_Test`

- [ ] **Step 1: Create the test map under `/Game/FrostMage/Levels`**
- [ ] **Step 2: Add test caster, several targets, and basic lighting/camera support**
- [ ] **Step 3: Wire debug UI display for the isolated test flow if a safe attachment point exists inside the test map only**
- [ ] **Step 4: Save the map**

### Task 6: Compile, PIE, Log Review, and Diff Summary

**Files:**
- Verify: `/Game/FrostMage/**`

- [ ] **Step 1: Compile all created Blueprints**
- [ ] **Step 2: Compile all created Widgets**
- [ ] **Step 3: Save all modified and created assets**
- [ ] **Step 4: Run PIE smoke test in `L_Frostbolt_Test`**
- [ ] **Step 5: Inspect Output Log for warnings and errors related to FrostMage assets**
- [ ] **Step 6: Review git status and summarize created assets and validation results**
