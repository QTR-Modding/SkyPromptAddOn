# SkyPrompt Addon

https://www.nexusmods.com/skyrimspecialedition/mods/148703

---

## Text Background (effect 2)

Add this to a theme JSON:

```json
{
  "special_effect": 2
}
```

Draws a translucent black rectangle behind each prompt's text, excluding its
button icon. It follows the text's rotation and fade in every layout, including
all visible List rows. Requires a SkyPrompt build with per-text effect support.

Optional: `special_integers[0]` sets the background color as a packed ABGR
integer. For example, `"special_integers": [2147483648]` is black with alpha
128, the default. The other parameter arrays are unused by this effect.
Effect 0 remains disabled; effect 1 remains Viny Arcs.

For host integration, include only `SkyPrompt/AddOns.hpp` and call the per-text
overload before drawing the label:

```cpp
SkyPrompt::AddOns::RenderSpecialEffect(specials, drawList, textMin, textMax, angle, alpha);
```

`textMin` and `textMax` are the unrotated text bounds in screen pixels. `angle`
is rotation in radians around their center; `alpha` is the prompt fade from 0
to 1. Their defaults are 0 and 1. Keep the existing four-argument overload for
effects drawn once around the whole prompt group.

## Installation (via vcpkg)

Add to your `vcpkg.json`:

```json
"dependencies": [
  "skyprompt-addon"
]
```

In your `CMakeLists.txt`:

```cmake
find_path(SkyPromptAPI_INCLUDE_DIRS "SkyPrompt/AddOns.hpp")
target_include_directories(your_target PRIVATE ${SkyPromptAddOn_INCLUDE_DIRS})
```

This is a header-only library; no linking is needed.

To use the SkyPrompt Addon port locally, copy the cmake/ folder from the SkyPrompt Addon repository into your project:

```markdown
your-project/
└── cmake/
    └── ports/
        └── skyprompt-addon/
            ├── portfile.cmake
            └── vcpkg.json

```
