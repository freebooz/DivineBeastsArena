# Frost Texture Pack Design

Date: 2026-06-26

## Goal

Create a consistent production-ready VFX texture pack for an original frost magic projectile spell intended for direct use in Unreal Engine Niagara systems and materials.

The pack must satisfy these constraints:

- All textures use a pure black background.
- The style is stylized AAA fantasy game VFX.
- The palette is blue-white icy with high contrast.
- The textures must be additive-material friendly where appropriate.
- The pack must contain no text, logo, character, or environment content.
- The output should prioritize import-ready production use over concept-art presentation.

## Asset List

The pack contains eight separate textures:

1. `Frost core glow` at `1024x1024`
2. `Frost ribbon trail` at `2048x512`
3. `Ice shard mask sheet` at `1024x1024`
4. `Frost mist noise` at `1024x1024`
5. `Frost sparkle sheet` at `1024x1024`
6. `Frost impact ring` at `1024x1024`
7. `Frost crack decal` at `1024x1024`
8. `Frost impact flipbook` at `2048x2048`

## Visual Direction

The pack uses a single shared shape language:

- Sharp crystalline edges for energy cores, shards, cracks, and impact accents
- Soft cold vapor falloff for mist, peripheral glow, and impact fade
- Bright cold-white highlights only at the hottest focal points
- Dominant icy blue and pale blue-gray in the secondary body shapes
- Clean silhouettes that remain readable in motion and at reduced screen size

The textures must avoid visual cues associated with fire, lightning, nebulae, or environmental illustration. These are gameplay VFX textures, not scene art.

## Functional Grouping

### Emissive-Primary Textures

These textures are expected to work immediately in Additive or Translucent emissive materials:

- `Frost core glow`
- `Frost ribbon trail`
- `Frost impact ring`
- `Frost impact flipbook`

Their design priority is strong center brightness, a clean black surround, and minimal need for post-cleanup after import.

### Mask-Forward Textures

These textures should preserve shape information and brightness layering for downstream material control:

- `Ice shard mask sheet`
- `Frost mist noise`
- `Frost crack decal`

They still keep the pack's icy blue-white read, but their internal value structure matters more than maximal emissive intensity.

### Hybrid Texture

`Frost sparkle sheet` must satisfy both use cases:

- bright enough for direct emissive sprite use
- clear enough per-cell for Niagara SubUV particle readability

## Per-Asset Design Requirements

### 1. Frost Core Glow

- A centered blue-white magical ice energy orb
- Very bright sharp luminous center
- Radial energy bloom around the center
- Secondary icy filaments or crystalline turbulence near the core
- Soft outer falloff into pure black
- Composition must remain circular and centered for billboard use

Primary use:

- projectile head glow
- cast pulse center
- impact flash overlay

### 2. Frost Ribbon Trail

- A horizontal icy energy streak flowing left to right
- Bright leading edge with stronger white-blue concentration
- Fading tail with tapered dissipation
- Wispy frost filaments and directional motion detail
- Clear center mass with feathered edge breakup
- Black margins preserved above and below the ribbon

Primary use:

- Niagara ribbon
- stretched projectile trail sprite

### 3. Ice Shard Mask Sheet

- Multiple isolated angular crystal splinters
- Each shard must read as a distinct reusable element
- Shapes should vary in length, angle, and thickness
- Edges are sharp and crystalline, not rounded
- The background remains pure black
- Layout should avoid heavy overlap so single shards can be sampled or cropped if needed

Primary use:

- impact burst sprites
- shard cards
- masked distortion or emissive overlays

### 4. Frost Mist Noise

- Soft pale blue-gray cold vapor
- Smoky wisps with subtle directional turbulence
- Tileable in both axes
- No obvious focal center
- No hard isolated shapes that would reveal tiling too quickly
- Value variation should support panning, UV distortion, and opacity modulation

Primary use:

- ambient cold vapor
- impact fade mist
- cast residue
- panning opacity noise

### 5. Frost Sparkle Sheet

- A `4x4` sprite sheet containing 16 individual cells
- Each cell contains a small blue-white sparkle, snowflake accent, or star-like particle
- Shapes should vary across cells but remain stylistically consistent
- The cells must be evenly spaced with no border lines
- Each sprite must stay centered enough for predictable SubUV playback
- Individual sprites should read clearly at small scale

Primary use:

- Niagara sparkle particles
- hit accents
- secondary cast twinkles

### 6. Frost Impact Ring

- Top-down circular icy shockwave ring
- Transparent or black center with energy concentrated on the ring band
- Cracked frozen edge detail around the circumference
- Strong radial read from a top-down camera
- Slight asymmetry is acceptable, but the overall ring must stay centered and circular

Primary use:

- ground impact plane
- impact flash ring
- radial burst overlay

### 7. Frost Crack Decal

- Top-down branching ice crack mask
- White and pale blue cracks radiating from the center region
- Cracks should break outward in multiple directions with natural variance
- Outer edges should dissipate cleanly into black
- The image should feel decal-ready rather than like a painted terrain texture

Primary use:

- impact decal
- freeze-hit ground mark
- dissolve reveal mask

### 8. Frost Impact Flipbook

- A `4x4` sprite sheet containing 16 sequential frames
- The animation starts from a small flash
- Expands into a crystalline frost burst
- Transitions into dissipating mist and residual sparkle
- Every frame must keep a consistent center point
- No border lines or frame dividers
- Frame-to-frame growth should be readable in SubUV playback without jitter

Primary use:

- main projectile hit animation
- detonation burst

## Unreal Engine Usage Assumptions

This pack is designed around typical Unreal Niagara and material workflows:

- `Frost core glow` for billboard sprite heads and burst flashes
- `Frost ribbon trail` for ribbon renderers or stretched directional sprites
- `Ice shard mask sheet` for impact splinters and crystal fragment particles
- `Frost mist noise` for panning opacity and soft atmospheric breakup
- `Frost sparkle sheet` for SubUV sparkle particles
- `Frost impact ring` for top-down impact planes and radial overlays
- `Frost crack decal` for deferred or translucent decal-style freeze marks
- `Frost impact flipbook` for a one-shot SubUV impact burst

The pack assumes the lowest-friction hookup is:

- black background sampled directly
- RGB used as emissive source where appropriate
- brightness and opacity shaped in the material instance
- optional recoloring through tint multiplication

## Production Rules

- The black background must remain visually pure so additive blending behaves cleanly.
- Shapes must remain readable under bloom and screen-space compression.
- The pack must feel authored as one spell family, not as unrelated icy images.
- The images must favor clean functional silhouette over painterly noise.
- The sprite sheets must be aligned for reliable SubUV indexing.
- The mist texture must remain genuinely tileable, not merely centered and soft.

## Out of Scope

This design does not yet define:

- Unreal import settings
- texture compression settings
- channel packing variants
- Niagara system assembly
- material graph implementation

Those are downstream implementation steps after the texture pack itself is approved.
