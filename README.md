# SkyPrompt Addon

https://www.nexusmods.com/skyrimspecialedition/mods/148703

---

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
