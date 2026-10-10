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
| 6 | Separator | SkyPromptAddOn |
| 7 | Frame | SkyPromptAddOn |

IDs 3-5 are occupied even though they are not in the AddOn's `EffectID` enum.
The next unused ID is currently **8**. Check this list and open contributions
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

## Effects 6 and 7: Separator and Frame

This theme draws dividing lines between client groups and a frame around the
displayed prompts:

```json
{
    "name": "Grouped Prompts",
    "special_effects": [
        { "special_effect": 6 },
        { "special_effect": 7 }
    ]
}
```

A client group is a run of neighboring prompts with the same `clientID`.
For example, one client sends Talk and Trade, and another sends Follow and Wait.
Separator draws one line between those groups. Frame surrounds all four prompts,
including their icons. Each effect can also be used on its own.

The prompts must already share the display. Mod authors can use
[handshakes](https://github.com/QTR-Modding/SkyPromptAPI/wiki/SkyPrompt-API-Tutorial-(cpp)#compatibility-handshakes)
to allow different clients to appear together. Their shared theme supplies the
effect settings.

### Effect 6: Separator

Separator works in Vertical, List, and Horizontal layouts. It is disabled in
Radial and Diamond. In List, it separates only the currently visible client groups.

```json
{
    "special_effect": 6,
    "special_floats": [2.0, 12.0, 6.0],
    "special_integers": [2164260863]
}
```

Add this object to `special_effects`. It gives each line a thickness of 2 pixels,
leaves at least 12 pixels of space on each side, and extends each end by 6 pixels.
The color is white with roughly 50% opacity.

| Array | Position | Setting | Default |
| --- | --- | --- | --- |
| `special_floats` | 0 | Line thickness in screen pixels, from 0 to 100 | `2.0` |
| `special_floats` | 1 | Minimum space on each side of the line in screen pixels, from 0 to 500 | `12.0` |
| `special_floats` | 2 | Extension of each end in screen pixels, from -500 to 500 | `0.0` |
| `special_integers` | 0 | Line color, including opacity | `2164260863` (white, alpha 128) |

The theme's normal prompt spacing is kept if it already provides more room.
Negative extension shortens the line. Thickness 0 or a transparent color disables
it without adding space. There are no boolean or string settings.

### Effect 7: Frame

Frame draws an outline around the visible prompt area in every layout. It sizes
itself to the icons and text. Prompts attached to different objects each have
their own frame.

```json
{
    "special_effect": 7,
    "special_floats": [8.0, 12.0, 4.0, 2.0],
    "special_integers": [2164260863]
}
```

Add this object to `special_effects`. It leaves 8 pixels of padding on the left
and right and 12 on the top and bottom, rounds the corners with a 4-pixel radius,
and draws a 2-pixel white border with roughly 50% opacity.

| Array | Position | Setting | Default |
| --- | --- | --- | --- |
| `special_floats` | 0 | Horizontal padding per side in screen pixels, from -500 to 500 | `8.0` |
| `special_floats` | 1 | Vertical padding per side in screen pixels, from -500 to 500 | `8.0` |
| `special_floats` | 2 | Corner radius in screen pixels, from 0 to 500 | `4.0` |
| `special_floats` | 3 | Border thickness in screen pixels, from 0 to 100 | `2.0` |
| `special_integers` | 0 | Border color, including opacity | `2164260863` (white, alpha 128) |

Negative padding moves the border inward without moving the prompts. If either
dimension becomes zero or negative, the frame is not drawn. Thickness 0 disables
it; corner radius 0 gives square corners. Frame has no fill, boolean settings,
or string settings. Both effects follow the prompts' fade.

### Color Format

Use the in-game color picker and export the theme to obtain color values for JSON.
They are unsigned packed ABGR integers, written in decimal. ABGR specifies the
alpha, blue, green, and red bytes from highest to lowest. Alpha runs from 0
(transparent) to 255 (opaque). Text Background, Separator, and Frame use it;
Viny Arcs supplies its own opacity.

## Adding More Effects

Effects are C++ drawing functions. Their code and dispatch currently live in
[`include/SkyPrompt/AddOns.hpp`](https://github.com/QTR-Modding/SkyPromptAddOn/blob/main/include/SkyPrompt/AddOns.hpp).

1. Choose an unused ID from the [shared list](#overview), and add a named entry to `SpecialEffects::EffectID`.
2. Add your drawing function inside `SpecialEffects`. Keep its helpers together with the effect.
3. Call it from the appropriate entry point below. Effects that need group boundaries or full display bounds also need SkyPrompt to supply that geometry.
4. Document its ID, parameter order, defaults, and a theme example on this page. Include those changes in your AddOn pull request.

SkyPrompt passes settings through `SpecialsView`: `integers`, `floats`, `bools`,
and `strings` correspond to the theme's `special_*` arrays. A theme may omit
any of them, so check a value exists before reading it and provide a default.

### Choose where to draw

| Effect placement | Existing entry point | Example |
| --- | --- | --- |
| Once around the prompt group | `RenderSpecialEffect(specials, center, size, resolutionScale)` | Viny Arcs |
| Behind each text label | `RenderSpecialEffect(specials, drawList, textMin, textMax, angle, alpha)` | Text Background |
| Between client groups | `SpecialEffects::Separator(specials).Draw(drawList, lineStart, lineEnd, alpha)` | Separator |
| Around the full prompt area | `SpecialEffects::Frame(specials).Draw(drawList, minimum, maximum, alpha)` | Frame |

For text effects, the bounds are unrotated screen-pixel coordinates. Rotate
around the text's center by `angle` (radians) and multiply opacity by `alpha`
(0-1) to follow the prompt. Text Background shows how to do both.

The four-argument `RenderSpecialEffect` overload receives the group's anchor,
icon size, and resolution scale, but not an individual prompt's progress or
activation state.

Separator and Frame use screen-pixel coordinates. SkyPrompt calculates their
line endpoints and display bounds after laying out the visible prompts. Their
constructors read `SpecialsView` once; the parsed spacing and padding are also
available to the layout code. `Draw` applies extension or padding and multiplies
the configured opacity by `alpha` (0-1).

<details>
<summary>Example: a circle around the prompt group</summary>

This example uses ID 8; it is not an effect included with SkyPrompt.
Confirm the ID is still unused before adding it.

Add `kOutlineCircle = 8` to `SpecialEffects::EffectID`, then add this function
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
            "special_effect": 8,
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
