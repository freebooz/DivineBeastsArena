# 3D Character Presentation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace UMG Viewport character previews on character select/create screens with a real 3D frontend presentation stage rendered in the game world behind transparent UI.

**Architecture:** Add a focused world actor, `ADBACharacterPresentationActor`, that owns the preview mesh, lights, camera, and input-driven rotation. Character select/create widgets stop spawning `UViewport` and instead find or spawn the stage actor in the current world, update its zodiac, and rotate it from pointer/touch drag input.

**Tech Stack:** Unreal Engine 5.8 C++, UUserWidget input overrides, AActor world presentation stage, existing DBA zodiac mesh/material assets.

---

### Task 1: Add World Presentation Actor

**Files:**
- Create: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h`
- Create: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.cpp`

- [ ] Create an actor with scene root, skeletal mesh, camera, three directional lights, and sky light.
- [ ] Reuse the existing zodiac mesh/material lookup behavior from `ADBACharacterPreviewActor`.
- [ ] Add public methods `SetPreviewZodiac`, `AddPreviewYaw`, and `ActivatePresentationCamera`.

### Task 2: Replace UMG Viewport Preview Calls

**Files:**
- Modify: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h`
- Modify: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.cpp`
- Modify: `Source/DivineBeastsArena/Public/GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.h`
- Modify: `Source/DivineBeastsArena/Private/GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.cpp`

- [ ] Replace `InitializePreviewViewport` / `DestroyPreviewViewport` responsibilities with world-stage lifecycle helpers.
- [ ] Keep function names where practical to minimize header churn, but route logic to `ADBACharacterPresentationActor`.
- [ ] Keep drag/touch input and rotate the world actor instead of a viewport-spawned actor.

### Task 3: Make UI Transparent Over 3D Stage

**Files:**
- Modify: select/create widget C++ as needed.

- [ ] Stop creating preview-stage overlay images inside the UMG tree.
- [ ] Leave blueprint panels intact so artists can manually move panels to the left/right.
- [ ] Ensure input mode still shows the mouse and allows UI clicks.

### Task 4: Verify

**Commands:**
- Run: `D:\UnrealEngine-5.8.0-release\Engine\Build\BatchFiles\Build.bat DivineBeastsArenaEditor Win64 Development D:\DivineBeastsArenaPlatform\DBA_GameClient\DivineBeastsArena.uproject -WaitMutex -FromMsBuild`
- Expected: `Result: Succeeded`

- [ ] Launch editor after compile and confirm it stays alive.
- [ ] Manual PIE test: login -> character select -> create character; model should be in the actual world, not a UMG rectangle.

