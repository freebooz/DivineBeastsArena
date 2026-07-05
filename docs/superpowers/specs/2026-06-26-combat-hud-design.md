# Combat HUD Design

## Goal
Create a new combat HUD widget set under `/Game/UI/CombatHUD` that provides a battle-ready player status panel, skill bar, and interaction prompt without modifying existing assets outside that directory.

## Scope
- Only create or modify assets under `/Game/UI/CombatHUD`.
- Do not modify existing assets under `/Game/UI/Arena`, `/Game/UI/Common`, or any other content path.
- Analyze existing HUD and attribute structures with Monolith before implementation.
- Create and modify widgets with UnrealMotionGraphicsMCP-compatible tooling available in this session.
- Compile all affected widgets, check for layout overlap, run PIE smoke validation, and inspect Output Log.

## Existing Context
- Existing arena UI assets live mainly under `/Game/UI/Arena/HUD` and `/Game/UI/Arena/AbilityBar`.
- Current Monolith read-only inspection exposes `DBABattleAttributeSet` and `DBAHeroGrowthAttributeSet`.
- Existing widget-tree introspection for current Arena HUD assets does not expose reusable child structure through the adapter, so the new combat HUD should be built as isolated assets rather than attempting structural cloning.

## User-Confirmed Design Decisions
- Integration mode: create new child widgets under `/Game/UI/CombatHUD` for later integration into the existing Arena HUD, without directly modifying current Arena HUD assets in this task.
- Skill bar scale: `3` active skills, `1` ultimate slot, `1` interaction slot.
- Status panel information density: standard combat layout with health bar, stamina bar, level/class badge area, and reserved status icon area.

## Recommended Architecture
Create four widget assets under `/Game/UI/CombatHUD`:

1. `WBP_DBA_CombatStatusPanel`
2. `WBP_DBA_CombatSkillBar`
3. `WBP_DBA_InteractionPrompt`
4. `WBP_DBA_CombatHUDRoot`

Responsibilities:

- `WBP_DBA_CombatStatusPanel`
  - Shows player name
  - Shows level/class badge area
  - Shows health and stamina bars
  - Reserves space for status icons

- `WBP_DBA_CombatSkillBar`
  - Owns five slots in the order: skill 1, skill 2, skill 3, ultimate, interaction
  - Supports icon, key label, cooldown/progress overlay, and selected/emphasis styling

- `WBP_DBA_InteractionPrompt`
  - Shows interaction key, short action label, and optional contextual hint
  - Supports hidden/visible states cleanly without affecting surrounding layout

- `WBP_DBA_CombatHUDRoot`
  - Owns top-level canvas layering and anchoring
  - Places status panel in the lower-left safe area
  - Places skill bar centered at the bottom
  - Places interaction prompt near lower-right or lower-center without overlapping the skill bar

## Layout Rules
- Use safe anchoring suitable for common gameplay resolutions.
- Avoid overlap between the status panel and skill bar by keeping them in separate anchor regions.
- Keep interaction prompt offset from the skill bar and away from the screen edge.
- Prefer a clean, readable combat style over decorative density.
- Reserve enough spacing so later integration of buffs, cast bars, or target frame does not force immediate redesign.

## Data and Binding Strategy
- Health and stamina should be represented with named variables and binding-ready widget structure.
- The UI should assume future connection to battle attributes from `DBABattleAttributeSet`.
- If direct attribute binding is supported by the widget tooling, bind through exposed variables or binding endpoints created inside the new widgets.
- If direct binding is not supported by the current tooling path, create a binding-ready structure only:
  - progress bars
  - numeric text fields
  - named widget variables
  - clearly separated display regions

This keeps the task within the asset-path restriction while still making the widgets integration-ready.

## Visual Design Direction
- Standard combat readability
- Strong foreground contrast for bars and slot icons
- Clear visual hierarchy:
  - health most prominent
  - stamina secondary
  - ultimate visually emphasized over normal skills
  - interaction prompt distinct from combat skill slots

## Asset Change Strategy
- Only one widget is modified at a time.
- After each widget creation or modification:
  - compile that widget
  - inspect layout for overlap risks
- After all widgets are complete:
  - compile all CombatHUD widgets
  - run PIE smoke validation
  - inspect Output Log
  - produce git diff summary if git access is available

## Verification Requirements
- Compile every widget under `/Game/UI/CombatHUD`.
- Validate there is no obvious layout overlap in the assembled root HUD.
- Run PIE smoke test after the root widget is complete.
- Inspect Output Log for widget compile issues, runtime warnings, and layout-related errors.

## Constraints and Risks
- Current session tooling exposes `mcp__umg`, `mcp__unreal`, and `mcp__monolith`; the task should map UnrealMotionGraphicsMCP and VibeUE responsibilities onto those exposed tools.
- Git operations are currently blocked by repository safe-directory ownership rules, so commit and diff reporting may require environment adjustment if raw git output is needed.
- Arena HUD integration is intentionally deferred; this task produces new sub-widgets only.

## Success Criteria
- New assets exist only under `/Game/UI/CombatHUD`.
- Combat HUD provides:
  - health bar
  - stamina bar
  - level/class badge region
  - status icon reserve area
  - skill bar with 3 active slots, 1 ultimate slot, 1 interaction slot
  - interaction prompt widget
  - root layout widget
- All created widgets compile successfully.
- PIE smoke validation runs after assembly.
- Final report includes:
  - added assets
  - modified assets
  - verification results
  - git diff summary or explicit note if git summary is blocked by environment state
