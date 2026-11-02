# Building and Installation Guide

This document explains how to build the **OBS Audio Router Plugin** to produce the final standalone `obs-audio-router.dll` binary and install it into OBS Studio.

---

## 1. Development Prerequisites

To build native C++20 plugins with Qt6 for OBS Studio on Windows:

1. **Visual Studio 2022** (with the *"Desktop development with C++"* workload selected).
2. **CMake** (version 3.20 or newer).
3. **OBS Studio SDK & Qt6**:
   - The OBS Studio installation located at `C:\Program Files\obs-studio` contains runtime DLLs (`obs.dll`, `obs-frontend-api.dll`, `Qt6Core.dll`, `Qt6Widgets.dll`).
   - For compilation headers and export libraries, download the matching release headers from the [OBS Studio GitHub Releases](https://github.com/obsproject/obs-studio/releases) or use the official [obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate).

---

## 2. Step-by-Step Compilation (CMake + MSVC)

Open a Developer PowerShell for VS 2022 (or standard PowerShell) and run:

```powershell
# 1. Navigate to the plugin directory
cd c:\Users\Jvgso\Projetos\apps\obs-audio-router-plugin

# 2. Generate build configuration using the Visual Studio generator
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

# 3. Compile the Release binary
cmake --build build --config Release
```

The compiled binary will be located at:
`build\Release\obs-audio-router.dll`

---

## 3. 1-File Installation (Standalone)

1. **Copy the binary:**
   Copy `obs-audio-router.dll` directly into:
   ```text
   C:\Program Files\obs-studio\obs-plugins\64bit\
   ```
   *(or user directory `%APPDATA%\obs-studio\plugins\obs-audio-router\bin\64bit\`)*

2. **Launch OBS Studio:**
   OBS will automatically load the plugin at startup.
   The **Audio Router** dock will be available under the top menu: **Docks -> Audio Router**.
   *(All translations and UI assets are self-contained inside `obs-audio-router.dll`, no extra folders needed).*
