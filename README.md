# OBS Audio Router Plugin

A native, ultra-lightweight plugin for [OBS Studio](https://obsproject.com/) that enables streamers and content creators to separate and route application audio into dedicated thematic channels, functioning similarly to dedicated hardware mixers with near-zero CPU and RAM overhead.

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
>    - Open top menu: **Docks (Painéis)** → **Audio Router**.
>    - Done! Your per-app audio mixer is ready to use.

---

## 🎯 Goal & Features

Eliminate complex external virtual audio cables. This plugin manages Windows audio sessions internally and routes them into categorized buses directly inside OBS.

- ⚡ **Ultra-Lightweight**: Built with native C++20 and Qt, consuming **less than 5 MB of RAM** and **~0.0% CPU**.
- ✨ **Smart Out-of-the-Box Auto-Routing**:
  - 🎮 **Game**: Automatically detects games and game launchers (`steam.exe`, `cs2.exe`, `valorant.exe`, `leagueclient.exe`, etc.).
  - 💬 **Voice / Chat**: Pre-configured for communication tools (`discord.exe`, `slack.exe`, `teamspeak.exe`, `whatsapp.exe`, etc.).
  - 🎵 **Media / Music**: Pre-configured for music players and streaming apps (`spotify.exe`, `vlc.exe`, `tidal.exe`, `music.ui.exe`, etc.).
  - ⚙️ **System**: Fallback channel for miscellaneous apps and Windows system sounds.
- 👂 **Stream Monitoring ("Ouvir Stream")**: One-click toggle in the top bar to listen in your headphones exactly to what is being broadcasted to the stream.
- 🎛️ **ChatMix Slider**: Fast attenuator slider between Game audio and Chat audio with double-click center reset.
- 🔄 **Real-Time WASAPI Session Tracking**: Instantly detects when any program starts outputting sound.
- 🖱️ **Drag & Drop Reassignment**: Move active applications between channel columns directly on the dock.
- 🎚️ **Dual Bus Mixing (Stream vs. Monitor)**: Independent volume sliders and mute controls for your live broadcast output and your headphones.
- 🌐 **Multi-Language Support (i18n)**: Fully translated into **English**, **Portuguese**, **Spanish**, **French**, and **Russian**.
- 💾 **Persistent User Rules**: Remembers custom application assignments across sessions in a lightweight JSON format.

---

## 📂 Repository Structure

```text
obs-audio-router-plugin/
├── CMakeLists.txt
├── README.md
├── BUILDING.md
├── data/
│   └── locale/
│       ├── en-US.ini
│       ├── pt-BR.ini
│       ├── es-ES.ini
│       ├── fr-FR.ini
│       └── ru-RU.ini
├── src/
│   ├── plugin-main.cpp
│   ├── plugin-macros.hpp
│   ├── plugin-loader.cpp
│   ├── core/
│   │   ├── audio-session-watcher.hpp / .cpp
│   │   ├── channel-audio-engine.hpp / .cpp
│   │   ├── routing-rules.hpp / .cpp
│   │   └── process-utils.hpp / .cpp
│   └── ui/
│       ├── audio-router-dock.hpp / .cpp
│       ├── chatmix-slider.hpp / .cpp
│       ├── channel-strip-widget.hpp / .cpp
│       ├── app-badge-widget.hpp / .cpp
│       └── localization-helper.hpp
└── tests/
    ├── test_i18n_and_rules.cpp
    └── test_chatmix_and_volumes.cpp
```
