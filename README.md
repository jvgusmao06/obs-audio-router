# OBS Audio Router Plugin

A native, ultra-lightweight plugin for [OBS Studio](https://obsproject.com/) that enables streamers and content creators to separate and route application audio into dedicated thematic channels, functioning similarly to dedicated hardware mixers with near-zero CPU and RAM overhead.

---

> [!IMPORTANT]
> ### 🚀 Super Easy Installation (Only 1 File: `obs-audio-router.dll`)
>
> 1. **Download**:
>    - Go to the **[Releases](../../releases)** tab.
>    - Download **`obs-audio-router.dll`** (or `obs-audio-router-windows-x64.zip`).
>
> 2. **Drop into your OBS plugins folder**:
>    - Close OBS Studio if open.
>    - Copy `obs-audio-router.dll` directly into:
>      ```text
>      C:\Program Files\obs-studio\obs-plugins\64bit\
>      ```
>
> 3. **Open OBS Studio**:
>    - Launch OBS Studio.
>    - Open top menu: **Docks** → **Audio Router**.
>    - Done! The plugin is 100% self-contained in that single `.dll` (embedded translations for English, Portuguese, Spanish, French, and Russian included).

---

## 🎯 Goal & Features

Eliminate complex external virtual audio cables. This plugin manages Windows audio sessions internally and routes them into categorized buses directly inside OBS.

- 📦 **100% Standalone Single DLL**: Zero external runtime files or drivers required. Just drop `obs-audio-router.dll` into OBS!
- ⚡ **Ultra-Lightweight**: Built with native C++20 and Qt, consuming **less than 5 MB of RAM** and **~0.0% CPU** (compared to 300MB–600MB+ consumed by background commercial software suites).
- ✨ **Smart Out-of-the-Box Auto-Routing**:
  - 🎮 **Game**: Automatically detects games (`cs2.exe`, `valorant.exe`, etc.). Game launchers (Steam, Epic, Riot Client) default to System to keep your game channel clean.
  - 💬 **Voice / Chat**: Pre-configured for communication tools (`discord.exe`, `slack.exe`, `teamspeak.exe`, `whatsapp.exe`, etc.).
  - 🎵 **Media / Music**: Pre-configured for music players and streaming apps (`spotify.exe`, `vlc.exe`, `tidal.exe`, `music.ui.exe`, etc.).
  - ⚙️ **System**: Fallback channel for miscellaneous apps and Windows system sounds.
- 👂 **Stream Monitoring ("Listen to Stream")**: One-click toggle in the top bar to listen in your headphones exactly to what is being broadcasted to the stream.
- 🎛️ **ChatMix Slider**: Fast attenuator slider between Game audio and Chat audio with double-click center reset.
- 🔄 **Real-Time WASAPI Session Tracking**: Instantly detects when any program starts outputting sound.
- 🖱️ **Drag & Drop Reassignment**: Move active applications between channel columns directly on the dock.
- 🚫 **Ignored Applications**: Exclude specific apps from routing with a simple right-click ("Ignore"), keeping them completely untouched and playing normally directly to Windows. OBS Studio itself is ignored out-of-the-box, and a top-bar dialog allows viewing and restoring ignored apps at any time.
- 🎚️ **Dual Bus Mixing (Stream vs. Monitor)**: Independent volume sliders and mute controls for your live broadcast output and your headphones.
- 🌐 **Embedded Multi-Language Support (i18n)**: Automatically adapts to OBS language (English, Portuguese, Spanish, French, and Russian) with zero external files needed.
- 💾 **Persistent User Rules**: Remembers custom application assignments across sessions in a lightweight JSON format.

---

## ⚠️ Known Architecture Limitations & Troubleshooting

### 1. The Windows WASAPI Loopback Pipeline
On Windows, the plugin utilizes the native per-process capture API (`AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK`). This API has an intrinsic Windows architectural characteristic:

1. **Pre-Loopback Attenuation:** Windows scales application audio frames by the session volume (`ISimpleAudioVolume`, controlled by the **🎧 Headphones** fader) **before** delivering them to the OBS loopback capture buffer.
2. **Impact on Muting / Low Volume:**
   - If an application renders directly to your physical headphones and you set its **Headphones fader to 0% (or Mute)**, Windows completely stops generating audio frames for that program on that endpoint.
   - Consequently, OBS receives pure silence. **In native, driverless Windows audio, it is impossible to mute sound in your physical headphones while keeping it at 100% on stream if both target the exact same hardware output.**
   - If the headphone fader is set to 20%, the signal captured by OBS will also have its dynamic range attenuated at the OS source.

### 2. Why Commercial Software (SteelSeries Sonar, Elgato Wave Link) Can Do This
Proprietary programs achieve absolute isolation (e.g., playing background music on stream with 0% in your headphones) because they install **custom kernel-mode audio drivers (`.sys`) signed with Microsoft EV/WHQL certificates**. They instantiate 4 to 5 virtual audio devices in Windows, force applications to output to those virtual devices at 100%, and the background software (consuming hundreds of megabytes of RAM) routes an attenuated copy back to your physical headphones.

Attempting to bundle generic virtual cables (such as VB-Cable) inside lightweight plugins introduces significant user friction:
- It creates multiple confusing virtual endpoints in the Windows sound menu (`CABLE Input`, `CABLE 16ch`).
- If not manually configured and monitored, it causes sudden, complete audio loss on the PC.
- It requires administrator privileges and may trigger sample-rate mismatches (e.g., 44.1 kHz vs 48 kHz).

### 3. Best Practices

To get the best experience from **OBS Audio Router**:

- **Standard Broadcast Mixing:** Keep application volumes in Windows and the **Headphones** faders at a comfortable listening level (70%–100%). Use the **Live** faders in Audio Router to balance game, music, and Discord levels for your viewers.
- **Playing Audio on Stream Without Hearing It in Your Headphones (Silent in Ears):**
  - In Windows, go to **Settings → System → Sound → Volume mixer**.
  - Route the output of that specific application (e.g., Spotify or browser) to any secondary unused endpoint (such as your monitor's HDMI audio, optical out, or `Steam Streaming Speakers`).
  - Because the app outputs at 100% volume to that secondary endpoint, OBS continues capturing its process cleanly for the stream, while your primary physical headphones remain completely silent.
- **"Listen to Stream" Button:** When you want to verify the exact mix being broadcast to your audience (Live mix), toggle the **`🎧 Listen to Stream`** button in the top bar. Toggle it off when you want the **Headphones** faders to resume direct control over your personal listening.

---

## 📂 Repository Structure

```text
obs-audio-router-plugin/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── BUILDING.md
├── .gitignore
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
    ├── test_channel_audio_engine.cpp
    ├── test_chatmix_and_volumes.cpp
    └── test_routing_rules.cpp
```

---

## 🛠️ Building from Source

See [BUILDING.md](BUILDING.md) for detailed instructions on compiling with CMake, Ninja, and Qt 6 on Windows 64-bit.

---

## 📄 License

Distributed under the [MIT License](LICENSE).
