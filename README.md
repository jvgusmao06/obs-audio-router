# OBS Audio Router Plugin

A native, ultra-lightweight plugin for [OBS Studio](https://obsproject.com/) that enables streamers and content creators to separate and route application audio into dedicated thematic channels, functioning similarly to **SteelSeries Sonar** and **Voicemeeter** with near-zero CPU and RAM overhead.

---

> [!IMPORTANT]
> ### 🚀 Super Easy Installation (1 Single Step!)
>
> 1. **Download**:
>    - Go to the **[Releases](../../releases)** tab.
>    - Download `obs-audio-router-windows-x64.zip`.
>
> 2. **Extract directly into your OBS folder**:
>    - Close OBS Studio if open.
>    - Extract the `.zip` directly into your main OBS Studio installation directory:
>      ```text
>      C:\Program Files\obs-studio\
>      ```
>      *(The zip already contains the exact internal folder hierarchy. Windows will merge it automatically in 1 click).*
>
> 3. **Open OBS Studio**:
>    - Launch OBS Studio.
>    - Open top menu: **Docks (Painéis)** → **Audio Router (Sonar Style)**.
>    - Done! Your per-app audio mixer is ready to use.

---

## 🎯 Goal & Features

Eliminate complex external virtual audio cables. This plugin manages Windows audio sessions internally and routes them into categorized buses directly inside OBS.

- ⚡ **Ultra-Lightweight**: Built with native C++20 and Qt, consuming **less than 5 MB of RAM** and **~0.0% CPU**.
- ✨ **Smart Out-of-the-Box Auto-Routing**:
  - 🎮 **Game**: Automatically detects games and game launchers (`steam.exe`, `cs2.exe`, `valorant.exe`, `leagueclient.exe`, etc.).
  - 💬 **Voice / Chat**: Pre-configured for communication tools (`discord.exe`, `slack.exe`, `teamspeak.exe`, `whatsapp.exe`, etc.).
  - 🎵 **Media / Music**: Pre-configured for music players and streaming apps (`spotify.exe`, `vlc.exe`, `tidal.exe`, `music.ui.exe`, etc.).
  - 🌐 **System**: Fallback channel for miscellaneous apps and Windows system sounds.
- 🔄 **Real-Time WASAPI Session Tracking**: Instantly detects when any program starts outputting sound.
- 🖱️ **Drag & Drop Reassignment**: Move active applications between channel columns directly on the dock.
- 🎚️ **Dual Bus Mixing (Stream vs. Monitor)**: Independent volume sliders and mute controls for your live broadcast output and your headphones.
- 🌐 **Multi-Language Support (i18n)**: Fully translated into **English**, **Portuguese**, **Spanish**, **French**, and **Russian**.
- 💾 **Persistent User Rules**: Remembers custom application assignments across sessions in a lightweight JSON format.

---

## 📂 Repository Structure

```text
obs-audio-router-plugin/
├── .github/workflows/    # Automated CI/CD release workflow
├── data/locale/          # Language files (en-US, pt-BR, es-ES, fr-FR, ru-RU)
├── dist/                 # Pre-built release binaries and distribution packages
├── docs/                 # Architecture design and planning specs
├── src/
│   ├── core/             # WASAPI loopback, process helpers, routing rules engine
│   └── ui/               # Native Qt6 dock widget, drag-and-drop channel columns
├── tests/                # Unit tests for rules engine, WASAPI, and i18n
├── CMakeLists.txt        # Build system configuration
└── README.md             # Project documentation
```

---

## 🌐 Localization

The plugin detects your OBS Studio language setting automatically:
- 🇺🇸 **English** (`en-US` - Default)
- 🇧🇷 **Portuguese** (`pt-BR`)
- 🇪🇸 **Spanish** (`es-ES`)
- 🇫🇷 **French** (`fr-FR`)
- 🇷🇺 **Russian** (`ru-RU`)

---

## 🔨 Building from Source (Developers)

```powershell
# 1. Configure the project
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

# 2. Build Release DLL
cmake --build build --config Release
```

---

## 📄 License

Distributed under the [GPL-2.0 License](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html), fully compatible with OBS Studio and `libobs`.
