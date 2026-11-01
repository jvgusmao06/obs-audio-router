# OBS Audio Router Plugin

A high-performance, native plugin for [OBS Studio](https://obsproject.com/) that enables streamers and content creators to separate and route application audio into dedicated thematic channels, functioning similarly to **SteelSeries Sonar** and **Voicemeeter** without requiring any external virtual audio drivers.

---

> [!IMPORTANT]
> ### 🚀 Super Easy 3-Step Installation
>
> 1. **Download**:
>    - Go to the **[Releases](../../releases)** tab.
>    - Download `obs-audio-router.zip`.
>
> 2. **Drop into OBS plugins folder**:
>    - Press <kbd>Win</kbd> + <kbd>R</kbd>, paste the following path and press **Enter**:
>      ```text
>      %APPDATA%\obs-studio\plugins
>      ```
>      *(If the `plugins` folder doesn't exist yet, simply create it).*
>    - Extract the downloaded `.zip` directly there so you have the `obs-audio-router` folder inside it:
>      ```text
>      %APPDATA%\obs-studio\plugins\obs-audio-router\
>      ├── bin\64bit\obs-audio-router.dll
>      └── data\locale\ (all language files)
>      ```
>
> 3. **Open OBS Studio**:
>    - Launch OBS Studio.
>    - Open the top menu: **Docks** → **Audio Router (Sonar Style)**.
>    - Done! Your audio router dock is ready to use.

---

## 🎯 Goal & Features

Eliminate complex external virtual audio cables. This plugin manages Windows audio sessions internally and routes them into categorized buses directly inside OBS.

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

## 🛠️ Architecture & Tech Stack

- **Plugin API**: `libobs` and `obs-frontend-api` (OBS Studio 29+)
- **Audio Capture Subsystem**: Windows Audio Session API (WASAPI `IAudioSessionManager2`, `IAudioSessionNotification`, and Process Loopback)
- **UI Framework**: Native Qt6 (`QDockWidget`, `QDrag`, `QDropEvent`)
- **Core Language**: C++20 with CMake build system

---

## 🌐 Localization

The plugin detects your OBS Studio language setting automatically:
- 🇺🇸 **English** (`en-US` - Default)
- 🇧🇷 **Portuguese** (`pt-BR`)
- 🇪🇸 **Spanish** (`es-ES`)
- 🇫🇷 **French** (`fr-FR`)
- 🇷🇺 **Russian** (`ru-RU`)

Language files are located in `data/locale/`. Contributions for additional languages are welcome!

---

## 🔨 Building from Source (For Developers)

If you wish to compile the plugin yourself from source code, check the [Building Guide](BUILDING.md).

```powershell
# 1. Configure the project
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

# 2. Build Release DLL
cmake --build build --config Release
```

---

## 🤝 Contributing

Feedback, bug reports, and pull requests are welcome! Feel free to open an issue or submit a PR.

---

## 📄 License

Distributed under the [GPL-2.0 License](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html), fully compatible with OBS Studio and `libobs`.
