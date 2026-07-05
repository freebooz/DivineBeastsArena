# Frost Texture Pack Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Generate and deliver an import-ready frost projectile VFX texture pack for Unreal Niagara and materials, with eight consistent blue-white icy textures on pure black backgrounds.

**Architecture:** Produce a tightly constrained set of standalone source textures as `.png` files under a new FrostMage source-art folder, using one controlled generation pass per asset family and verifying dimensions, sheet layout assumptions, and background cleanliness after each output. Keep the pack production-oriented by separating emissive-primary assets from mask-forward assets, and avoid modifying any existing Unreal binary assets.

**Tech Stack:** Codex `image_gen`, workspace file management, PowerShell verification commands, Git diff review

---

## File Structure

### New Files

- `docs/superpowers/specs/2026-06-26-frost-texture-pack-design.md`
  - Approved design spec for the texture pack.
- `docs/superpowers/plans/2026-06-26-frost-texture-pack.md`
  - This implementation plan.
- `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_CoreGlow_1024.png`
  - Projectile core glow source texture.
- `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_RibbonTrail_2048x512.png`
  - Directional frost ribbon trail source texture.
- `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_IceShardSheet_1024.png`
  - Ice shard mask sheet source texture.
- `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_MistNoise_1024.png`
  - Tileable frost mist noise source texture.
- `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_SparkleSheet_4x4_1024.png`
  - 4x4 sparkle sprite sheet source texture.
- `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_ImpactRing_1024.png`
  - Frost impact ring source texture.
- `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_CrackDecal_1024.png`
  - Frost crack decal source texture.
- `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_ImpactFlipbook_4x4_2048.png`
  - 4x4 frost impact flipbook source texture.

### Modified Files

- None. This plan only adds new non-binary source textures and planning docs.

## Task 1: Create Output Folder Baseline

**Files:**
- Create: `DBA_GameClient/Content/FrostMage/Textures/SourceArt/`

- [ ] **Step 1: Create the source-art folder**

Run:

```powershell
New-Item -ItemType Directory -Force 'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt'
```

Expected:

```text
Directory created or already exists with no error.
```

- [ ] **Step 2: Verify the folder path**

Run:

```powershell
Get-ChildItem 'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures'
```

Expected:

```text
A SourceArt directory is listed.
```

## Task 2: Generate Emissive Core and Trail

**Files:**
- Create: `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_CoreGlow_1024.png`
- Create: `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_RibbonTrail_2048x512.png`

- [ ] **Step 1: Generate the core glow texture**

Prompt to use:

```text
Create a production-ready game VFX texture on a pure black background only. Centered blue-white magical frost energy orb with a sharp luminous white-blue core, radial icy glow, crystalline turbulence near the center, soft falloff into black, high contrast, additive-material friendly, stylized AAA fantasy VFX, no text, no logo, no character, no environment. 1024x1024.
```

Expected:

```text
One square texture with a centered frost orb and clean black margins.
```

- [ ] **Step 2: Save the generated core glow to the exact output path**

Output file:

```text
E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_CoreGlow_1024.png
```

- [ ] **Step 3: Generate the ribbon trail texture**

Prompt to use:

```text
Create a production-ready game VFX texture on a pure black background only. Horizontal icy energy streak flowing left to right, bright blue-white leading edge, fading tail, wispy frost filaments, readable central streak, feathered edges, high contrast, additive-material friendly, stylized AAA fantasy VFX, no text, no logo, no character, no environment. 2048x512.
```

Expected:

```text
One wide texture with a bright directional leading edge and a clean dissipating tail.
```

- [ ] **Step 4: Save the generated ribbon trail to the exact output path**

Output file:

```text
E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_RibbonTrail_2048x512.png
```

- [ ] **Step 5: Verify both image dimensions**

Run:

```powershell
Add-Type -AssemblyName System.Drawing;
$files = @(
  'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_CoreGlow_1024.png',
  'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_RibbonTrail_2048x512.png'
);
foreach ($file in $files) {
  $img = [System.Drawing.Image]::FromFile($file);
  [PSCustomObject]@{ Path = $file; Width = $img.Width; Height = $img.Height };
  $img.Dispose();
}
```

Expected:

```text
T_Frost_CoreGlow_1024.png reports 1024x1024.
T_Frost_RibbonTrail_2048x512.png reports 2048x512.
```

## Task 3: Generate Mask-Forward Shards and Mist

**Files:**
- Create: `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_IceShardSheet_1024.png`
- Create: `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_MistNoise_1024.png`

- [ ] **Step 1: Generate the ice shard mask sheet**

Prompt to use:

```text
Create a production-ready game VFX texture on a pure black background only. Multiple isolated translucent icy crystal splinters with angular sharp shapes, varied sizes and orientations, clean spacing, high contrast, mask-friendly readability, blue-white icy palette, stylized AAA fantasy VFX, no text, no logo, no character, no environment. 1024x1024.
```

Expected:

```text
One square texture containing multiple distinct shard elements that do not collapse into a single cluster.
```

- [ ] **Step 2: Save the shard sheet to the exact output path**

Output file:

```text
E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_IceShardSheet_1024.png
```

- [ ] **Step 3: Generate the frost mist noise texture**

Prompt to use:

```text
Create a production-ready seamless tileable game VFX texture on a pure black background only. Soft pale blue-gray cold vapor, smoky wisps, subtle turbulence, no focal center, high contrast but soft value transitions, suitable for opacity noise and emissive mist layering, stylized AAA fantasy VFX, no text, no logo, no character, no environment. 1024x1024.
```

Expected:

```text
One square tileable mist texture with no obvious center and no obvious seam cues.
```

- [ ] **Step 4: Save the mist noise to the exact output path**

Output file:

```text
E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_MistNoise_1024.png
```

- [ ] **Step 5: Verify both image dimensions**

Run:

```powershell
Add-Type -AssemblyName System.Drawing;
$files = @(
  'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_IceShardSheet_1024.png',
  'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_MistNoise_1024.png'
);
foreach ($file in $files) {
  $img = [System.Drawing.Image]::FromFile($file);
  [PSCustomObject]@{ Path = $file; Width = $img.Width; Height = $img.Height };
  $img.Dispose();
}
```

Expected:

```text
Both textures report 1024x1024.
```

## Task 4: Generate Sparkle Sheet and Impact Ring

**Files:**
- Create: `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_SparkleSheet_4x4_1024.png`
- Create: `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_ImpactRing_1024.png`

- [ ] **Step 1: Generate the sparkle sprite sheet**

Prompt to use:

```text
Create a production-ready 4x4 sprite sheet on a pure black background only. Sixteen evenly spaced small blue-white frost sparkles, snowflake-like glints, and star particles, one per cell, consistent style, no borders, readable at small size, additive-friendly, stylized AAA fantasy VFX, no text, no logo, no character, no environment. 1024x1024.
```

Expected:

```text
One square 4x4 sheet with distinct centered sparkle sprites in each cell and no frame dividers.
```

- [ ] **Step 2: Save the sparkle sheet to the exact output path**

Output file:

```text
E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_SparkleSheet_4x4_1024.png
```

- [ ] **Step 3: Generate the impact ring texture**

Prompt to use:

```text
Create a production-ready game VFX texture on a pure black background only. Top-down circular icy shockwave ring with a transparent black center, bright blue-white ring energy, cracked frozen edge detail, clean radial read, high contrast, additive-material friendly, stylized AAA fantasy VFX, no text, no logo, no character, no environment. 1024x1024.
```

Expected:

```text
One square top-down ring texture with a centered circular band and preserved dark center.
```

- [ ] **Step 4: Save the impact ring to the exact output path**

Output file:

```text
E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_ImpactRing_1024.png
```

- [ ] **Step 5: Verify both image dimensions**

Run:

```powershell
Add-Type -AssemblyName System.Drawing;
$files = @(
  'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_SparkleSheet_4x4_1024.png',
  'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_ImpactRing_1024.png'
);
foreach ($file in $files) {
  $img = [System.Drawing.Image]::FromFile($file);
  [PSCustomObject]@{ Path = $file; Width = $img.Width; Height = $img.Height };
  $img.Dispose();
}
```

Expected:

```text
Both textures report 1024x1024.
```

## Task 5: Generate Crack Decal and Impact Flipbook

**Files:**
- Create: `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_CrackDecal_1024.png`
- Create: `DBA_GameClient/Content/FrostMage/Textures/SourceArt/T_Frost_ImpactFlipbook_4x4_2048.png`

- [ ] **Step 1: Generate the frost crack decal**

Prompt to use:

```text
Create a production-ready top-down game VFX decal texture on a pure black background only. Branching ice crack mask radiating from the center, white and pale blue fracture lines, decal-ready shape, fading outer edges, high contrast, mask-friendly readability, stylized AAA fantasy VFX, no text, no logo, no character, no environment. 1024x1024.
```

Expected:

```text
One square decal texture with centered radiating crack structure and clean black outer area.
```

- [ ] **Step 2: Save the crack decal to the exact output path**

Output file:

```text
E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_CrackDecal_1024.png
```

- [ ] **Step 3: Generate the 4x4 impact flipbook**

Prompt to use:

```text
Create a production-ready 4x4 sprite sheet on a pure black background only. Sixteen sequential frames of a frost magic impact explosion growing from a small blue-white flash into a crystalline icy burst, then fading into mist and residual sparkles, consistent center in every frame, no borders, additive-material friendly, stylized AAA fantasy VFX, no text, no logo, no character, no environment. 2048x2048.
```

Expected:

```text
One square 4x4 sheet with 16 readable sequential frames and no frame dividers.
```

- [ ] **Step 4: Save the impact flipbook to the exact output path**

Output file:

```text
E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_ImpactFlipbook_4x4_2048.png
```

- [ ] **Step 5: Verify both image dimensions**

Run:

```powershell
Add-Type -AssemblyName System.Drawing;
$files = @(
  'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_CrackDecal_1024.png',
  'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt\T_Frost_ImpactFlipbook_4x4_2048.png'
);
foreach ($file in $files) {
  $img = [System.Drawing.Image]::FromFile($file);
  [PSCustomObject]@{ Path = $file; Width = $img.Width; Height = $img.Height };
  $img.Dispose();
}
```

Expected:

```text
T_Frost_CrackDecal_1024.png reports 1024x1024.
T_Frost_ImpactFlipbook_4x4_2048.png reports 2048x2048.
```

## Task 6: Run Visual and Packaging Verification

**Files:**
- Verify: `DBA_GameClient/Content/FrostMage/Textures/SourceArt/*.png`

- [ ] **Step 1: Verify the expected file set exists**

Run:

```powershell
Get-ChildItem 'E:\work\Game\DivineBeastsArena\DBA_GameClient\Content\FrostMage\Textures\SourceArt' -Filter '*.png' | Select-Object -ExpandProperty Name
```

Expected:

```text
Exactly these eight files are listed:
T_Frost_CoreGlow_1024.png
T_Frost_RibbonTrail_2048x512.png
T_Frost_IceShardSheet_1024.png
T_Frost_MistNoise_1024.png
T_Frost_SparkleSheet_4x4_1024.png
T_Frost_ImpactRing_1024.png
T_Frost_CrackDecal_1024.png
T_Frost_ImpactFlipbook_4x4_2048.png
```

- [ ] **Step 2: Visually inspect each texture for background cleanliness and composition**

Inspection checklist:

```text
- Background reads as pure black with no accidental gray haze outside the intended effect.
- Blue-white palette stays consistent across the pack.
- Ribbon is horizontally directional.
- Mist has no obvious centered subject.
- Sparkle sheet reads as a 4x4 grid without borders.
- Impact flipbook reads as a centered 16-frame sequence without jitter.
- Crack decal and shard sheet preserve clean silhouette information.
```

- [ ] **Step 3: Review the Git diff summary**

Run:

```powershell
git status --short
```

Expected:

```text
Only the new source-art PNG files and the new planning/spec files for this task appear as additions related to the texture pack.
```

## Self-Review

### Spec Coverage

- The plan creates all eight textures listed in the approved spec.
- The plan preserves the pure black background rule.
- The plan respects the emissive-primary and mask-forward grouping through prompt wording.
- The plan preserves sprite sheet requirements for the sparkle sheet and impact flipbook.
- The plan produces direct Unreal-usable `.png` source textures without touching existing `.uasset` content.

### Placeholder Scan

- No `TODO`, `TBD`, or deferred implementation placeholders remain.
- Every asset has an exact output file path.
- Every generation step has an exact prompt and expected result.

### Scope Check

- Scope is limited to source texture generation and verification.
- Unreal import, compression settings, and material hookup remain explicitly out of scope, matching the approved spec.
