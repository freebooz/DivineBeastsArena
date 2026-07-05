# Combat HUD Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a new modular combat HUD widget set under `/Game/UI/CombatHUD` with a status panel, skill bar, interaction prompt, and root layout, without modifying assets outside that path.

**Architecture:** Create four isolated UMG assets under `/Game/UI/CombatHUD`. Build each widget independently, compile it immediately after editing, and assemble them only at the root level after all child widgets are stable. Use Monolith for read-only inspection and UI edits, map UnrealMotionGraphicsMCP responsibilities to the exposed `ui_query`/`mcp__umg` tools, and use editor PIE/log validation at the end.

**Tech Stack:** Unreal Engine UMG, Monolith MCP (`ui_query`, `editor_query`, `describe_query`, `gas_query`), exposed UMG compiler tool (`mcp__umg.compile_blueprint`)

---

## File Structure

**Create:**
- `/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel`
- `/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar`
- `/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt`
- `/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot`
- `docs/superpowers/specs/2026-06-26-combat-hud-design.md`
- `docs/superpowers/plans/2026-06-26-combat-hud.md`

**Read-only references:**
- `/Game/UI/Arena/HUD/WBP_DBA_ArenaHUDRoot`
- `/Game/UI/Arena/HUD/WBP_DBA_PlayerUnitFrame`
- `/Game/UI/Arena/AbilityBar/WBP_DBA_AbilityBar`
- `DBABattleAttributeSet`
- `DBAHeroGrowthAttributeSet`

**Do not modify:**
- Any asset outside `/Game/UI/CombatHUD`

### Task 1: Reconfirm Read-Only Baseline

**Files:**
- Read: `/Game/UI/Arena/HUD/WBP_DBA_ArenaHUDRoot`
- Read: `/Game/UI/Arena/HUD/WBP_DBA_PlayerUnitFrame`
- Read: `/Game/UI/Arena/AbilityBar/WBP_DBA_AbilityBar`

- [ ] **Step 1: Confirm widget-tree and attribute-set inspection inputs still resolve**

Use:
```json
{"action":"get_widget_tree","params":{"asset_path":"/Game/UI/Arena/HUD/WBP_DBA_ArenaHUDRoot"}}
{"action":"get_widget_tree","params":{"asset_path":"/Game/UI/Arena/HUD/WBP_DBA_PlayerUnitFrame"}}
{"action":"get_widget_tree","params":{"asset_path":"/Game/UI/Arena/AbilityBar/WBP_DBA_AbilityBar"}}
{"action":"list_attribute_sets","params":{}}
```

Expected:
- Arena references resolve without writing
- Attribute sets include `DBABattleAttributeSet`

- [ ] **Step 2: Record the implementation assumption**

Record in working notes:
```text
Arena widget structure is reference-only; CombatHUD assets will be built independently under /Game/UI/CombatHUD.
```

- [ ] **Step 3: Stop if any write would be required outside CombatHUD**

Expected:
```text
No asset outside /Game/UI/CombatHUD is created or modified during this plan.
```

### Task 2: Create `WBP_DBA_CombatStatusPanel`

**Files:**
- Create: `/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel`

- [ ] **Step 1: Create the Widget Blueprint**

Use:
```json
{"action":"create_widget_blueprint","params":{"save_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","parent_class":"UserWidget","root_widget":"CanvasPanel"}}
```

Expected:
- Widget asset is created and saved under `/Game/UI/CombatHUD`

- [ ] **Step 2: Add the main panel containers**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"Border","widget_name":"StatusCard","parent_name":"CanvasPanel","anchor_preset":"bottom_left","position":{"x":24,"y":-220},"size":{"x":420,"y":196},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"VerticalBox","widget_name":"StatusStack","parent_name":"StatusCard","compile":false}}
```

Then:
```json
{"action":"set_slot_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_name":"StatusCard","alignment":{"x":0,"y":1},"compile":false}}
```

Expected:
- A bottom-left status card and vertical content stack exist

- [ ] **Step 3: Add identity row widgets**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"HorizontalBox","widget_name":"IdentityRow","parent_name":"StatusStack","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"TextBlock","widget_name":"PlayerNameText","parent_name":"IdentityRow","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"TextBlock","widget_name":"LevelClassBadgeText","parent_name":"IdentityRow","compile":false}}
```

Set:
```json
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_name":"PlayerNameText","property_name":"Text","value":"Player"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_name":"LevelClassBadgeText","property_name":"Text","value":"Lv. 01  Vanguard"}}
```

Expected:
- Name and badge row is visible and readable

- [ ] **Step 4: Add health bar region**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"TextBlock","widget_name":"HealthLabelText","parent_name":"StatusStack","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"ProgressBar","widget_name":"HealthBar","parent_name":"StatusStack","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"TextBlock","widget_name":"HealthValueText","parent_name":"StatusStack","compile":false}}
```

Set:
```json
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_name":"HealthLabelText","property_name":"Text","value":"Health"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_name":"HealthValueText","property_name":"Text","value":"1250 / 1250"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_name":"HealthBar","property_name":"Percent","value":"1.0"}}
```

Expected:
- Health label, bar, and numeric value are present

- [ ] **Step 5: Add stamina bar region**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"TextBlock","widget_name":"StaminaLabelText","parent_name":"StatusStack","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"ProgressBar","widget_name":"StaminaBar","parent_name":"StatusStack","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"TextBlock","widget_name":"StaminaValueText","parent_name":"StatusStack","compile":false}}
```

Set:
```json
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_name":"StaminaLabelText","property_name":"Text","value":"Stamina"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_name":"StaminaValueText","property_name":"Text","value":"100 / 100"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_name":"StaminaBar","property_name":"Percent","value":"1.0"}}
```

Expected:
- Stamina label, bar, and numeric value are present

- [ ] **Step 6: Add status icon reserve row**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"HorizontalBox","widget_name":"StatusIconRow","parent_name":"StatusStack","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"Image","widget_name":"StatusIconSlotA","parent_name":"StatusIconRow","size":{"x":24,"y":24},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"Image","widget_name":"StatusIconSlotB","parent_name":"StatusIconRow","size":{"x":24,"y":24},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel","widget_class":"Image","widget_name":"StatusIconSlotC","parent_name":"StatusIconRow","size":{"x":24,"y":24},"compile":false}}
```

Expected:
- Three visible reserve icon slots exist

- [ ] **Step 7: Compile and inspect the widget**

Use:
```json
{"action":"compile_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel"}}
```

Then:
```json
{"blueprint_name":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel"}
```

Expected:
- Compile succeeds with no blocking errors

### Task 3: Create `WBP_DBA_CombatSkillBar`

**Files:**
- Create: `/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar`

- [ ] **Step 1: Create the Widget Blueprint**

Use:
```json
{"action":"create_widget_blueprint","params":{"save_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","parent_class":"UserWidget","root_widget":"CanvasPanel"}}
```

- [ ] **Step 2: Add the centered skill bar container**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"Border","widget_name":"SkillBarFrame","parent_name":"CanvasPanel","anchor_preset":"bottom_center","position":{"x":0,"y":-36},"size":{"x":760,"y":132},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"HorizontalBox","widget_name":"SkillSlotRow","parent_name":"SkillBarFrame","compile":false}}
```

Then:
```json
{"action":"set_slot_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"SkillBarFrame","alignment":{"x":0.5,"y":1},"compile":false}}
```

- [ ] **Step 3: Add the five slot widgets**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"Border","widget_name":"SkillSlot01","parent_name":"SkillSlotRow","size":{"x":96,"y":96},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"Border","widget_name":"SkillSlot02","parent_name":"SkillSlotRow","size":{"x":96,"y":96},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"Border","widget_name":"SkillSlot03","parent_name":"SkillSlotRow","size":{"x":96,"y":96},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"Border","widget_name":"UltimateSlot","parent_name":"SkillSlotRow","size":{"x":112,"y":112},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"Border","widget_name":"InteractSlot","parent_name":"SkillSlotRow","size":{"x":96,"y":96},"compile":false}}
```

- [ ] **Step 4: Add key labels**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"TextBlock","widget_name":"SkillKey01","parent_name":"SkillSlot01","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"TextBlock","widget_name":"SkillKey02","parent_name":"SkillSlot02","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"TextBlock","widget_name":"SkillKey03","parent_name":"SkillSlot03","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"TextBlock","widget_name":"UltimateKey","parent_name":"UltimateSlot","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"TextBlock","widget_name":"InteractKey","parent_name":"InteractSlot","compile":false}}
```

Set:
```json
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"SkillKey01","property_name":"Text","value":"Q"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"SkillKey02","property_name":"Text","value":"E"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"SkillKey03","property_name":"Text","value":"R"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"UltimateKey","property_name":"Text","value":"F"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"InteractKey","property_name":"Text","value":"T"}}
```

- [ ] **Step 5: Add slot labels and cooldown placeholders**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"TextBlock","widget_name":"SkillLabel01","parent_name":"SkillSlot01","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"TextBlock","widget_name":"SkillLabel02","parent_name":"SkillSlot02","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"TextBlock","widget_name":"SkillLabel03","parent_name":"SkillSlot03","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"TextBlock","widget_name":"UltimateLabel","parent_name":"UltimateSlot","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_class":"TextBlock","widget_name":"InteractLabel","parent_name":"InteractSlot","compile":false}}
```

Set:
```json
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"SkillLabel01","property_name":"Text","value":"Skill 1"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"SkillLabel02","property_name":"Text","value":"Skill 2"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"SkillLabel03","property_name":"Text","value":"Skill 3"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"UltimateLabel","property_name":"Text","value":"Ultimate"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar","widget_name":"InteractLabel","property_name":"Text","value":"Interact"}}
```

- [ ] **Step 6: Compile and inspect the widget**

Use:
```json
{"action":"compile_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar"}}
```

Then:
```json
{"blueprint_name":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar"}
```

Expected:
- Five-slot layout compiles
- Ultimate slot is visibly emphasized

### Task 4: Create `WBP_DBA_InteractionPrompt`

**Files:**
- Create: `/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt`

- [ ] **Step 1: Create the Widget Blueprint**

Use:
```json
{"action":"create_widget_blueprint","params":{"save_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","parent_class":"UserWidget","root_widget":"CanvasPanel"}}
```

- [ ] **Step 2: Add the prompt frame and content row**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","widget_class":"Border","widget_name":"PromptFrame","parent_name":"CanvasPanel","anchor_preset":"bottom_right","position":{"x":-44,"y":-160},"size":{"x":280,"y":72},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","widget_class":"HorizontalBox","widget_name":"PromptRow","parent_name":"PromptFrame","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","widget_class":"TextBlock","widget_name":"PromptKeyText","parent_name":"PromptRow","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","widget_class":"VerticalBox","widget_name":"PromptTextStack","parent_name":"PromptRow","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","widget_class":"TextBlock","widget_name":"PromptActionText","parent_name":"PromptTextStack","compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","widget_class":"TextBlock","widget_name":"PromptHintText","parent_name":"PromptTextStack","compile":false}}
```

Then:
```json
{"action":"set_slot_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","widget_name":"PromptFrame","alignment":{"x":1,"y":1},"compile":false}}
```

- [ ] **Step 3: Set the prompt copy**

Use:
```json
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","widget_name":"PromptKeyText","property_name":"Text","value":"T"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","widget_name":"PromptActionText","property_name":"Text","value":"Interact"}}
{"action":"set_widget_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt","widget_name":"PromptHintText","property_name":"Text","value":"Pick up or use nearby object"}}
```

- [ ] **Step 4: Compile and inspect the widget**

Use:
```json
{"action":"compile_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt"}}
```

Then:
```json
{"blueprint_name":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt"}
```

Expected:
- Prompt compiles and remains visually separate from the skill bar region

### Task 5: Create `WBP_DBA_CombatHUDRoot`

**Files:**
- Create: `/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot`

- [ ] **Step 1: Create the Widget Blueprint**

Use:
```json
{"action":"create_widget_blueprint","params":{"save_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot","parent_class":"UserWidget","root_widget":"CanvasPanel"}}
```

- [ ] **Step 2: Add child widget placeholders or references**

Use:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot","widget_class":"NamedSlot","widget_name":"StatusPanelHost","parent_name":"CanvasPanel","anchor_preset":"bottom_left","position":{"x":24,"y":-24},"size":{"x":420,"y":196},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot","widget_class":"NamedSlot","widget_name":"SkillBarHost","parent_name":"CanvasPanel","anchor_preset":"bottom_center","position":{"x":0,"y":-24},"size":{"x":760,"y":132},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot","widget_class":"NamedSlot","widget_name":"InteractionPromptHost","parent_name":"CanvasPanel","anchor_preset":"bottom_right","position":{"x":-44,"y":-160},"size":{"x":280,"y":72},"compile":false}}
```

Then:
```json
{"action":"set_slot_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot","widget_name":"StatusPanelHost","alignment":{"x":0,"y":1},"compile":false}}
{"action":"set_slot_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot","widget_name":"SkillBarHost","alignment":{"x":0.5,"y":1},"compile":false}}
{"action":"set_slot_property","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot","widget_name":"InteractionPromptHost","alignment":{"x":1,"y":1},"compile":false}}
```

Expected:
- Root establishes the final anchor map and overlap-safe regions

- [ ] **Step 3: If child widget embedding is supported directly, replace placeholders with concrete widget instances**

Use one of:
```json
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot","widget_class":"WBP_DBA_CombatStatusPanel","parent_name":"CanvasPanel","anchor_preset":"bottom_left","position":{"x":24,"y":-24},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot","widget_class":"WBP_DBA_CombatSkillBar","parent_name":"CanvasPanel","anchor_preset":"bottom_center","position":{"x":0,"y":-24},"compile":false}}
{"action":"add_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot","widget_class":"WBP_DBA_InteractionPrompt","parent_name":"CanvasPanel","anchor_preset":"bottom_right","position":{"x":-44,"y":-160},"compile":false}}
```

Fallback:
```text
Keep host slots if direct widget-class embedding is not supported by the current MCP surface.
```

- [ ] **Step 4: Compile and inspect the root widget**

Use:
```json
{"action":"compile_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot"}}
```

Then:
```json
{"blueprint_name":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot"}
```

Expected:
- Root compiles and reflects non-overlapping layout regions

### Task 6: Compile All CombatHUD Widgets

**Files:**
- Verify: `/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel`
- Verify: `/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar`
- Verify: `/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt`
- Verify: `/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot`

- [ ] **Step 1: Compile each widget in sequence**

Use:
```json
{"action":"compile_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel"}}
{"action":"compile_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar"}}
{"action":"compile_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt"}}
{"action":"compile_widget","params":{"asset_path":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot"}}
```

Expected:
- All four widgets return success or only non-blocking warnings

- [ ] **Step 2: Check for compile diagnostics**

Use:
```json
{"blueprint_name":"/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel"}
{"blueprint_name":"/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar"}
{"blueprint_name":"/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt"}
{"blueprint_name":"/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot"}
```

Expected:
- No compile errors remain

### Task 7: Run PIE Smoke Validation

**Files:**
- Verify runtime behavior through current editor level or designated test map

- [ ] **Step 1: Start PIE smoke**

Use:
```json
{"action":"run_pie_smoke","params":{"duration":5,"on_compile_errors":"refuse","log_patterns":{"must_absent":["Blueprint Runtime Error","Accessed None","Ensure condition failed"],"warn":["warning"]}}}
```

Expected:
- Returns `session_id`
- PIE session starts without blocking compile issues

- [ ] **Step 2: Poll until completion**

Use:
```json
{"action":"poll_pie_smoke","params":{"session_id":"<returned-session-id>"}} 
```

Repeat until final status is returned.

Expected:
- Smoke session completes
- No blocking runtime issues are reported

- [ ] **Step 3: Search logs for HUD-related errors**

Use:
```json
{"action":"search_logs","params":{"pattern":"CombatHUD","limit":100}}
{"action":"search_logs","params":{"pattern":"Widget", "verbosity":"warning","limit":100}}
{"action":"search_logs","params":{"pattern":"Blueprint Runtime Error","limit":100}}
```

Expected:
- No critical CombatHUD runtime or widget-layout errors

### Task 8: Summarize Assets and Diff

**Files:**
- Summarize: `/Game/UI/CombatHUD/*`

- [ ] **Step 1: List created assets**

Expected:
```text
/Game/UI/CombatHUD/WBP_DBA_CombatStatusPanel
/Game/UI/CombatHUD/WBP_DBA_CombatSkillBar
/Game/UI/CombatHUD/WBP_DBA_InteractionPrompt
/Game/UI/CombatHUD/WBP_DBA_CombatHUDRoot
```

- [ ] **Step 2: List modified assets**

Expected:
```text
Only assets under /Game/UI/CombatHUD and documentation files under docs/superpowers.
```

- [ ] **Step 3: Attempt git diff summary**

Run:
```powershell
git status --short
git diff --stat
```

Expected:
- If git ownership is fixed, report diff summary
- If git remains blocked by safe-directory rules, report the exact environment limitation instead of fabricating a diff

- [ ] **Step 4: Deliver final report**

Final report must include:
```text
Added assets
Modified assets
Verification results
Output Log summary
Git diff summary or explicit git-blocked note
```
