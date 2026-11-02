# Sonar Dock Redesign Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Transform the OBS Audio Router dock into a modern SteelSeries Sonar-style mixer with dual faders (Headphones vs Stream), top ChatMix slider, direct automatic app allocation to channel strips, and fix the dock close/uncheck bug.

**Architecture:** Refactor `AudioRouterDock` from `QDockWidget` to native `QWidget` for proper OBS docking; add `ChatMixSlider` and `ChannelStripWidget` components with styled dual vertical faders and bottom app chip containers; connect WASAPI `ISimpleAudioVolume` to drive real-time Windows application volume for the Headphones mix and ChatMix.

**Tech Stack:** C++20, Qt6 (Widgets, Stylesheets, Drag & Drop), Windows WASAPI (`ISimpleAudioVolume`, `IAudioSessionControl2`), libobs (`obs-frontend-api`, `libobs`), CMake.

**Spec:** `docs/superpowers/specs/2026-11-02-sonar-dock-redesign-design.md`

## Global Constraints
- Target platform: Windows 10/11 x64, OBS Studio 28+ / 30+.
- Native C++20 and Qt6 only; no external heavy runtimes.
- Maintain memory overhead < 5 MB RAM.
- All UI strings must use `I18nHelper` / `loc()` for EN/PT localization.
- Code simplicity: follow Ponytail rules (ruthless minimalism, no premature abstractions).

## Review Focus
- **OBS Dock uncheck desync**: closing dock via OBS [X] must properly uncheck menu item in OBS *Docks* menu.
- **QSlider vertical collapse**: sliders must have explicit minimum height (120px) and complete stylesheet geometry so handles never collapse into dots.
- **ChatMix boundary clamping**: moving slider to full Game (-1.0) or full Chat (+1.0) must scale volumes between 0.0 and 1.0 without negative values or divide-by-zero.
- **Initial app volume sync**: opening an app while channel is at 20% or muted must apply 20%/mute immediately, preventing 100% audio blasting.
- **App chip channel reassignment**: moving an app to another channel updates `RoutingRules` and saves persistent JSON.

---

### Task 1: Core Engine - WASAPI Session Volume & ChatMix Logic

**Files:**
- Modify: `src/core/audio-session-watcher.hpp`
- Modify: `src/core/audio-session-watcher.cpp`
- Modify: `src/core/channel-audio-engine.hpp`
- Modify: `src/core/channel-audio-engine.cpp`
- Create: `tests/test_chatmix_and_volumes.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: Windows WASAPI `ISimpleAudioVolume` from `IAudioSessionControl`.
- Produces:
  - `AudioSessionWatcher::setProcessVolume(const std::string& processName, float volume)`
  - `AudioSessionWatcher::setProcessMute(const std::string& processName, bool mute)`
  - `ChannelAudioEngine::setChatMix(float factor)` (-1.0f to +1.0f)
  - `ChannelAudioEngine::getChatMix() const -> float`

- [ ] **Step 1: Write unit test for ChatMix math and volume calculation**

Create `tests/test_chatmix_and_volumes.cpp` testing that:
- When ChatMix = 0.0 (center), Game factor = 1.0, Chat factor = 1.0.
- When ChatMix = -0.5 (more Game), Game factor = 1.0, Chat factor = 0.5.
- When ChatMix = -1.0 (full Game), Game factor = 1.0, Chat factor = 0.0.
- When ChatMix = +0.5 (more Chat), Game factor = 0.5, Chat factor = 1.0.
- When ChatMix = +1.0 (full Chat), Game factor = 0.0, Chat factor = 1.0.

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir build -R test_chatmix` or compile and run executable.
Expected: FAIL (files/methods not yet created).

- [ ] **Step 3: Implement WASAPI volume controls and ChatMix in `AudioSessionWatcher` and `ChannelAudioEngine`**

In `audio-session-watcher.cpp`:
- Implement `setProcessVolume(processName, volume)`: query `ISimpleAudioVolume` from stored sessions matching process name and call `SetMasterVolume(volume, nullptr)`.
- Implement `setProcessMute(processName, mute)`: call `SetMute(mute, nullptr)`.

In `channel-audio-engine.cpp`:
- Implement `setChatMix(float factor)`: store `m_chatMix = std::clamp(factor, -1.0f, 1.0f)`. Recalculate and update `setMonitorVolume` for `GAME` and `CHAT`.
- In `setMonitorVolume(channel, volume)`: calculate effective gain considering `m_chatMix`. Call `AudioSessionWatcher::setProcessVolume` on all assigned processes.
- In `setMuted(channel, muted)`: call `AudioSessionWatcher::setProcessMute` on assigned processes.

- [ ] **Step 4: Run test to verify it passes**

Run: `ctest --test-dir build -R test_chatmix`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/core/ tests/ CMakeLists.txt
git commit -m "feat(core): implement WASAPI session volume control and ChatMix logic"
```

---

### Task 2: UI - Fix OBS Dock Base Class & Create `ChatMixSlider`

**Files:**
- Modify: `src/ui/audio-router-dock.hpp`
- Modify: `src/ui/audio-router-dock.cpp`
- Modify: `src/plugin-main.cpp`
- Create: `src/ui/chatmix-slider.hpp`
- Create: `src/ui/chatmix-slider.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `ChannelAudioEngine::setChatMix(float)`
- Produces:
  - `AudioRouterDock` inheriting from `QWidget` (instead of `QDockWidget`).
  - `ChatMixSlider : public QWidget` with signal `chatMixChanged(float factor)`.

- [ ] **Step 1: Refactor `AudioRouterDock` from `QDockWidget` to `QWidget`**

- In `src/ui/audio-router-dock.hpp`: change `: public QDockWidget` to `: public QWidget`.
- In `src/ui/audio-router-dock.cpp`: replace `setWidget(mainContainer)` with `setLayout(rootLayout)`. Remove any `QDockWidget` titlebar handling.
- Verify in `plugin-main.cpp` that `obs_frontend_add_dock_by_id` correctly receives `g_audioRouterDock` as `QWidget*`.

- [ ] **Step 2: Create `ChatMixSlider` widget**

In `src/ui/chatmix-slider.hpp` / `.cpp`:
- Horizontal layout:
  - Left: 🎮 Game icon (`QLabel`).
  - Center: `QSlider(Qt::Horizontal)` with range `[-100, 100]`, centered at 0.
  - Right: 💬 Chat icon (`QLabel`).
  - Centered label above slider: "CHATMIX" (`#8e9297`, bold, 10px uppercase).
- Double-click on slider resets to 0 (center 50/50).
- Slider stylesheet: sleek dark groove (`#202225`, height 4px), notched silver handle (`#dcddde`, 14x14px).
- Emits `chatMixChanged(val / 100.0f)`.

- [ ] **Step 3: Connect `ChatMixSlider` to `ChannelAudioEngine`**

In `AudioRouterDock::setupUi()`:
- Add `m_chatMixSlider = new ChatMixSlider(this)`.
- Connect `chatMixChanged` signal to `ChannelAudioEngine::instance().setChatMix`.

- [ ] **Step 4: Verify compilation**

Run: `cmake --build build --config Release`
Expected: Build succeeds with 0 errors.

- [ ] **Step 5: Commit**

```bash
git add src/ui/ src/plugin-main.cpp CMakeLists.txt
git commit -m "feat(ui): convert dock to QWidget and create ChatMixSlider"
```

---

### Task 3: UI - Modern Sonar `ChannelStripWidget` with Dual Faders & App Chips

**Files:**
- Create: `src/ui/channel-strip-widget.hpp`
- Create: `src/ui/channel-strip-widget.cpp`
- Modify: `src/ui/app-badge-widget.hpp`
- Modify: `src/ui/app-badge-widget.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: `ChannelType`, `ChannelAudioEngine`, `RoutingRules`
- Produces:
  - `ChannelStripWidget : public QFrame`
  - Methods:
    - `addApp(const QString& appName, uint32_t pid)`
    - `removeApp(const QString& appName)`
    - `clearApps()`
    - Signals: `appReassigned(const QString& appName, ChannelType newChannel)`

- [ ] **Step 1: Implement `ChannelStripWidget` layout & styles**

In `src/ui/channel-strip-widget.hpp` / `.cpp`:
- Frame style: dark rounded container (`background-color: #26282c; border: 1px solid #1e1f23; border-radius: 8px; min-width: 130px;`).
- **Header**:
  - Colored indicator dot / icon (Green `#00f0aa` for Game, Blue `#5865f2` for Chat, Pink `#eb459e` for Media, Yellow `#faa61a` for System).
  - Uppercase title: **GAME**, **CHAT**, **MEDIA**, **SYSTEM**.
- **Fader Section (Dual Faders)**:
  - Horizontal layout containing two vertical fader columns:
    - **Left (🎧 Fone)**:
      - Icon `🎧`, vertical `QSlider` (min-height: 120px, dark groove `#18191c`, handle with accent color), percentage label (e.g. `100%`), Mute button (`QPushButton`, checkable, speaker icon / "Mute").
      - Connected to `ChannelAudioEngine::setMonitorVolume` and `setMuted`.
    - **Right (📡 Stream)**:
      - Icon `📡`, vertical `QSlider` (min-height: 120px, dark groove `#18191c`, handle with accent color), percentage label (e.g. `100%`), Mute button (`QPushButton`, checkable, broadcast icon / "Mute").
      - Connected to `ChannelAudioEngine::setStreamVolume` and `setMuted`.

- [ ] **Step 2: Implement "Apps" section at the bottom of `ChannelStripWidget`**

- Below faders: label "Apps" (`font-size: 10px; color: #72767d; text-transform: uppercase;`).
- App container: `QScrollArea` or `QVBoxLayout` with `background-color: #1e1f23; border-radius: 6px; min-height: 60px; max-height: 110px;`.
- Drag and drop: accept drops from other channel strips (`dragEnterEvent`, `dropEvent`) to move apps between channels.

- [ ] **Step 3: Enhance `AppBadgeWidget` with Channel Switch Menu**

In `app-badge-widget.hpp` / `.cpp`:
- Visual style: sleek dark pill (`background-color: #36393f; border-radius: 4px; padding: 3px 6px; color: #ffffff; font-size: 11px;`).
- Clicking or right-clicking opens a quick `QMenu`:
  - "Move to Game"
  - "Move to Chat"
  - "Move to Media"
  - "Move to System"
- Triggering an option emits `reassignRequested(appName, targetChannel)`.

- [ ] **Step 4: Verify compilation**

Run: `cmake --build build --config Release`
Expected: Build succeeds with 0 errors.

- [ ] **Step 5: Commit**

```bash
git add src/ui/ CMakeLists.txt
git commit -m "feat(ui): implement Sonar ChannelStripWidget with dual faders and app chips"
```

---

### Task 4: UI Assembly & Automatic Allocation in `AudioRouterDock`

**Files:**
- Modify: `src/ui/audio-router-dock.hpp`
- Modify: `src/ui/audio-router-dock.cpp`

**Interfaces:**
- Consumes: `ChannelStripWidget`, `ChatMixSlider`, `AudioSessionWatcher`, `RoutingRules`
- Produces: Integrated main mixer UI with automatic session allocation.

- [ ] **Step 1: Reassemble `AudioRouterDock::setupUi()`**

- Remove old `activeAppsFrame` and `m_activeAppsLayout` (the top unassigned list).
- Add top toolbar:
  - Left: Title label "AUDIO ROUTER" with Sonar logo/accent.
  - Center: `ChatMixSlider`.
- Add main channels area:
  - `QHBoxLayout` containing 4 `ChannelStripWidget`s:
    - `ChannelType::GAME`
    - `ChannelType::CHAT`
    - `ChannelType::MEDIA`
    - `ChannelType::SYSTEM`
- Connect `appReassigned` signal from each strip to `onAppAssignedToChannel`.

- [ ] **Step 2: Update `onNewSessionDetected` for direct auto-allocation**

In `AudioRouterDock::onNewSessionDetected`:
- When a new process is detected:
  - Resolve its channel via `m_rules->resolveChannel(appName.toStdString())`.
  - Add app directly to the target `ChannelStripWidget` via `addApp(appName, pid)`.
  - Synchronize initial volume: apply current channel's Fone volume, mute state, and ChatMix to the new process immediately.

- [ ] **Step 3: Connect app reassignment and rule persistence**

In `AudioRouterDock::onAppAssignedToChannel`:
- Remove app from old channel strip.
- Add app to new channel strip.
- Save custom rule in `m_rules->setCustomRule(appName, channel)` and persist to JSON.
- Reapply new channel's volume settings to the process.

- [ ] **Step 4: Verify compilation**

Run: `cmake --build build --config Release`
Expected: Build succeeds with 0 errors.

- [ ] **Step 5: Commit**

```bash
git add src/ui/
git commit -m "feat(ui): integrate auto-allocation and ChatMix into AudioRouterDock"
```

---

### Task 5: End-to-End Build, Test Verification & Distribution Packaging

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `data/locale/*.ini` (ensure new strings like ChatMix and Fone/Stream are localized)
- Output: `dist/obs-audio-router-windows-x64.zip`

- [ ] **Step 1: Run unit tests**

Run: `ctest --test-dir build --output-on-failure`
Expected: 100% tests pass.

- [ ] **Step 2: Build Release binary**

Run: `cmake --build build --config Release`
Expected: `build/bin/Release/obs-audio-router.dll` generated successfully.

- [ ] **Step 3: Update `dist/` release package**

Copy updated `.dll` and locale files to `dist/obs-plugins/64bit/` and `dist/data/obs-plugins/obs-audio-router/locale/`.
Generate zip package: `dist/obs-audio-router-windows-x64.zip`.

- [ ] **Step 4: Commit**

```bash
git add CMakeLists.txt data/ dist/
git commit -m "chore: package release binaries and update locale strings"
```

