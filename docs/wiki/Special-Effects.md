# Special Effects

SkyPromptAddOn provides effects that themes can enable and combine.
This example adds Viny Arcs and a padded text background:

```json
{
    "name": "My Theme",
    "special_effects": [
        { "special_effect": 1 },
        { "special_effect": 2, "special_floats": [6.0, 2.0, 4.0] }
    ]
}
```

For creating, saving, and using a theme, see the
[theme guide](https://github.com/QTR-Modding/SkyPromptAPI/wiki/How-to-create-Themes%3F).
The settings below describe the effects supplied by this repository.

## Overview

Effect IDs are shared by SkyPrompt and SkyPromptAddOn. The full list is:

| ID | Effect | Implemented in |
| --- | --- | --- |
| 0 | No extra effect | Neither |
| 1 | Viny Arcs | SkyPromptAddOn |
| 2 | Text Background | SkyPromptAddOn |
| 3 | Progress Circle | SkyPrompt |
| 4 | List Indicators | SkyPrompt |
| 5 | Activation Pop | SkyPrompt |

IDs 3-5 are occupied even though they are not in the AddOn's `EffectID` enum.
The next unused ID is currently **6**. Check this list and open contributions
before choosing an ID; published IDs must keep their meaning.

Settings for effects 3-5 are in the
[theme guide's effect reference](https://github.com/QTR-Modding/SkyPromptAPI/wiki/How-to-create-Themes%3F#special-visual-effects).

## Effect 1: Layered Gradient Arcs

**Viny Arcs** draws three nested, glowing arcs around the prompt group.
The arcs span 36, 72, and 117 degrees from outermost to innermost. Each can be
colored and switched on or off separately.

<details>
<summary>Settings</summary>

Array positions start at 0. Missing values use the defaults below.
Write float values with a decimal point, such as `0.0`.

| Array | Position | Setting | Default |
| --- | --- | --- | --- |
| `special_integers` | 0 | Outer arc color | Gold: RGB 255, 204, 0 |
| `special_integers` | 1 | Middle arc color | Dark gold: RGB 200, 160, 0 |
| `special_integers` | 2 | Inner arc color | Dark gold: RGB 200, 160, 0 |
| `special_floats` | 0 | Horizontal offset in resolution-scaled pixels; positive moves right | `0.0` |
| `special_floats` | 1 | Vertical offset in resolution-scaled pixels; positive moves down | `0.0` |
| `special_bools` | 0 | Show inner arc | `true` |
| `special_bools` | 1 | Show middle arc | `true` |
| `special_bools` | 2 | Show outer arc | `true` |

Viny Arcs has no string settings. Its colors use the red, green, and blue
channels; the effect supplies its own gradient opacity. Use the switches to hide
arcs rather than setting a color's alpha to zero.

</details>

## Effect 2: Text Background

Draws a colored rectangle behind each prompt's text, without including its icon.
It follows the text's rotation and fade, including every visible row in List.

```json
{
    "name": "My Theme",
    "special_effects": [
        {
            "special_effect": 2,
            "special_floats": [6.0, -2.0, 4.0],
            "special_integers": [2147483648]
        }
    ]
}
```

This adds 6 pixels on both the left and right, removes 2 pixels from the top and
bottom, and rounds the corners with a 4-pixel radius. The color is black with
roughly 50% opacity.

<details>
<summary>Settings</summary>

| Array | Position | Setting | Default |
| --- | --- | --- | --- |
| `special_floats` | 0 | Horizontal padding in screen pixels, applied to each side | `0.0` |
| `special_floats` | 1 | Vertical padding in screen pixels, applied to each side | `0.0` |
| `special_floats` | 2 | Corner radius in screen pixels | `0.0` |
| `special_integers` | 0 | Background color, including opacity | `2147483648` (black, alpha 128) |

Padding changes the background, not the text or icon positions. Negative padding
shrinks it; if either dimension is zero or negative, the background is not drawn.
A zero or negative corner radius gives square corners. There are no boolean or
string settings.

</details>

### Color Format

Use the in-game color picker and export the theme to obtain color values for JSON.
They are unsigned packed ABGR integers, written in decimal. ABGR specifies the
alpha, blue, green, and red bytes from highest to lowest. Alpha runs from 0
(transparent) to 255 (opaque); Text Background uses it, while Viny Arcs supplies
its own opacity.

## Adding More Effects

Effects are C++ drawing functions. Their code and dispatch currently live in
[`include/SkyPrompt/AddOns.hpp`](https://github.com/QTR-Modding/SkyPromptAddOn/blob/main/include/SkyPrompt/AddOns.hpp).

1. Choose an unused ID from the [shared list](#overview), and add a named entry to `SpecialEffects::EffectID`.
2. Add your drawing function inside `SpecialEffects`. Keep its helpers together with the effect.
3. Call it from the appropriate `RenderSpecialEffect` overload below.
4. Document its ID, parameter order, defaults, and a theme example on this page. Include those changes in your AddOn pull request.

SkyPrompt passes settings through `SpecialsView`: `integers`, `floats`, `bools`,
and `strings` correspond to the theme's `special_*` arrays. A theme may omit
any of them, so check a value exists before reading it and provide a default.

### Choose where to draw

| Effect placement | Existing entry point | Example |
| --- | --- | --- |
| Once around the prompt group | `RenderSpecialEffect(specials, center, size, resolutionScale)` | Viny Arcs |
| Behind each text label | `RenderSpecialEffect(specials, drawList, textMin, textMax, angle, alpha)` | Text Background |

For text effects, the bounds are unrotated screen-pixel coordinates. Rotate
around the text's center by `angle` (radians) and multiply opacity by `alpha`
(0-1) to follow the prompt. Text Background shows how to do both.

Group effects receive the group's anchor, icon size, and resolution scale, but
not an individual prompt's progress or activation state.

<details>
<summary>Example: a circle around the prompt group</summary>

This example uses ID 6; it is not an effect included with SkyPrompt.
Confirm the ID is still unused before adding it.

Add `kOutlineCircle = 6` to `SpecialEffects::EffectID`, then add this function
inside `SpecialEffects`, before the dispatch functions:

```cpp
inline void OutlineCircle(ImDrawList* drawList, const ImVec2 center,
                          const float size, const float resolutionScale,
                          const SpecialsView& specials) {
    constexpr float defaultThickness = 2.0f;
    constexpr float radiusMultiplier = 2.0f;
    const auto color = specials.integers.empty()
        ? IM_COL32(255, 255, 255, 255) : specials.integers[0];
    const float thickness = specials.floats.empty()
        ? defaultThickness : std::max(specials.floats[0], 0.0f);
    if (thickness <= 0.0f) return;

    drawList->AddCircle(center, size * radiusMultiplier, color, 0,
                       thickness * resolutionScale);
}
```

Add a case to the four-argument `RenderSpecialEffect` switch:

```cpp
case SpecialEffects::kOutlineCircle:
    SpecialEffects::OutlineCircle(ImGui::GetBackgroundDrawList(), a_center,
                                  a_size, resScale, a_specials);
    break;
```

Once the updated AddOn is included in a SkyPrompt build, this theme enables it:

```json
{
    "name": "Outline Example",
    "special_effects": [
        {
            "special_effect": 6,
            "special_floats": [2.0],
            "special_integers": [4294967295]
        }
    ]
}
```

The float sets thickness in resolution-scaled pixels. The integer sets color;
`4294967295` is opaque white.

</details>

### Try and distribute the effect

SkyPromptAddOn is a header-only library compiled into SkyPrompt. A new effect
needs a SkyPrompt build using the updated AddOn dependency. Reload Themes reloads
JSON settings; it cannot add new C++ drawing code to an installed build.

Try the effect with omitted and custom parameters, in the layouts it supports,
and alongside other effects. For text effects, check rotation and fading too.

The Theme editor does not automatically discover AddOn names or parameter labels.
An effect enabled through JSON gets generic controls for its existing float and
integer values. Named menu entries and additional controls require separate
menu support in SkyPrompt; they are not required for the AddOn drawing code.

### Updating this wiki

Edit the pages in `docs/wiki/` and include them in your pull request.
Merging documentation changes into `main` updates the wiki automatically.
