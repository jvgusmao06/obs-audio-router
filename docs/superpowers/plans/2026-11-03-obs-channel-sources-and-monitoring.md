# OBS Channel Audio Sources & Dual-Bus Monitoring Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement native OBS per-application audio sources (`wasapi_process_output_capture`) inside `ChannelAudioEngine`, mapping the Live fader to stream volume output and the Fone fader to OBS audio monitoring, allowing applications to be heard 100% on stream/recording while 0% (silent) in the streamer's headphones without requiring virtual audio drivers.

**Architecture:** Extend `ChannelAudioEngine` to manage native OBS audio sources for each routed application process. Connect `setStreamVolume` and `setStreamMuted` to `obs_source_set_volume` and `obs_source_set_muted`. Connect `setMonitorVolume` and `setMonitorMuted` to `obs_source_set_monitoring_type` (`OBS_MONITORING_TYPE_NONE` when 0%/muted vs `OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT` when active).

**Tech Stack:** C++20, libobs (`obs_source_create`, `obs_source_set_volume`, `obs_source_set_monitoring_type`), Windows WASAPI, Qt6.

**Spec:** `docs/superpowers/specs/2026-11-03-obs-channel-sources-and-monitoring-design.md`

## Global Constraints

- Target platform: Windows 10/11 x64, OBS Studio 28+ / 30+.
- Native C++20, Qt6, and libobs only; zero external kernel audio drivers.
- Standalone single binary output: `obs-audio-router.dll`.
- Code simplicity: follow Ponytail rules (ruthless minimalism, minimal viable code, no unnecessary abstractions).
- All unit tests must pass before completing any task.

## Review Focus

1. **Monitor Mute vs Stream Output**: Setting Fone to 0% must set `OBS_MONITORING_TYPE_NONE` without zeroing stream volume (`obs_source_get_volume > 0.0f`).
2. **Process Exit Cleanup**: When an app terminates, `unassignProcess` must immediately call `obs_source_release` to prevent memory leaks or dead audio sources.
3. **Reassignment Across Channels**: Moving an app from Jogo to Mídia must update the source volume and monitoring type to reflect the target channel's faders immediately.
4. **Master Fader Scaling**: Scaling Master Live or Master Fone must propagate gain/monitoring state to all channel sources.
5. **Initial Assignment State**: Assigning a new process must immediately inherit the channel's current stream volume and monitoring type, avoiding volume bursts.

---

### Task 1: Per-Process OBS Audio Source Lifecycle in `ChannelAudioEngine`

**Files:**
- Modify: `src/core/channel-audio-engine.hpp`
- Modify: `src/core/channel-audio-engine.cpp`
- Test: `tests/test_channel_audio_engine.cpp`

**Interfaces:**
- Consumes: `libobs` (`obs_source_create`, `obs_source_release`, `obs_source_set_volume`, `obs_source_set_muted`, `obs_source_set_monitoring_type`, `obs_data_create`, `obs_data_set_string`, `obs_data_release`)
- Produces:
  - `ChannelAudioEngine::assignProcessToChannel(const std::string& processName, ChannelType channel)`
  - `ChannelAudioEngine::unassignProcess(const std::string& processName)`
  - `ChannelAudioEngine::getProcessSource(const std::string& processName) const -> obs_source_t*`

- [ ] **Step 1: Write the failing unit test for OBS source creation and release**

In `tests/test_channel_audio_engine.cpp`, add tests verifying:
- When a process is assigned to a channel, `engine.getProcessSource("spotify.exe")` returns a non-null `obs_source_t*`.
- When `engine.unassignProcess("spotify.exe")` is called, `engine.getProcessSource("spotify.exe")` returns `nullptr` and `obs_source_release` was invoked.

- [ ] **Step 2: Run test to verify it fails**

Run: `& C:\msys64\ucrt64\bin\g++.exe -std=c++20 tests/test_channel_audio_engine.cpp src/core/channel-audio-engine.cpp src/core/routing-rules.cpp src/core/audio-session-watcher.cpp src/core/process-utils.cpp -I src -lole32 -lpsapi -o test_engine.exe; .\test_engine.exe`
Expected: FAIL (compilation error: `getProcessSource` not defined).

- [ ] **Step 3: Implement per-process source tracking in `ChannelAudioEngine`**

In `src/core/channel-audio-engine.hpp`:
- Add `std::unordered_map<std::string, obs_source_t*> m_processSources;`
- Declare `obs_source_t* getProcessSource(const std::string& processName) const;`

In `src/core/channel-audio-engine.cpp`:
- Implement `getProcessSource` returning the source pointer or `nullptr`.
- In `assignProcessToChannel`: instantiate `wasapi_process_output_capture` using `obs_data_create` with `"window"` match for `processName`, store in `m_processSources`, apply initial stream volume, mute, and monitoring type.
- In `unassignProcess`: call `obs_source_release(it->second)` and erase from `m_processSources`.
- In `shutdownChannels`: release all sources in `m_processSources`.

- [ ] **Step 4: Run test to verify it passes**

Run: `& C:\msys64\ucrt64\bin\g++.exe -std=c++20 tests/test_channel_audio_engine.cpp src/core/channel-audio-engine.cpp src/core/routing-rules.cpp src/core/audio-session-watcher.cpp src/core/process-utils.cpp -I src -lole32 -lpsapi -o test_engine.exe; .\test_engine.exe`
Expected: PASS (`[SUCCESS] Todos os testes do motor de áudio passaram com êxito!`).

- [ ] **Step 5: Commit**

```bash
git add src/core/channel-audio-engine.hpp src/core/channel-audio-engine.cpp tests/test_channel_audio_engine.cpp
git commit -m "feat(core): implement per-process OBS audio source lifecycle in ChannelAudioEngine"
```

---

### Task 2: Dual-Bus Fader Control (Stream Volume vs Monitor Type)

**Files:**
- Modify: `src/core/channel-audio-engine.hpp`
- Modify: `src/core/channel-audio-engine.cpp`
- Test: `tests/test_chatmix_and_volumes.cpp`

**Interfaces:**
- Consumes: `m_processSources`, `obs_source_set_volume`, `obs_source_set_muted`, `obs_source_set_monitoring_type`
- Produces:
  - `ChannelAudioEngine::setStreamVolume(ChannelType channel, float volume)`
  - `ChannelAudioEngine::setMonitorVolume(ChannelType channel, float volume)`
  - `ChannelAudioEngine::setStreamMuted(ChannelType channel, bool muted)`
  - `ChannelAudioEngine::setMonitorMuted(ChannelType channel, bool muted)`

- [ ] **Step 1: Write the failing unit test for independent dual-bus control**

In `tests/test_chatmix_and_volumes.cpp`:
- Add mock trackers for `obs_source_set_monitoring_type` and `obs_source_set_volume`.
- Verify: when `setMonitorVolume(MEDIA, 0.0f)` is called and `setStreamVolume(MEDIA, 1.0f)` is called:
  - Monitoring type is set to `OBS_MONITORING_TYPE_NONE`.
  - Stream volume remains at `1.0f` (gain > 0.0f).
- Verify: when `setMonitorVolume(MEDIA, 0.8f)` is called:
  - Monitoring type is set to `OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT`.

- [ ] **Step 2: Run test to verify it fails**

Run: `& C:\msys64\ucrt64\bin\g++.exe -std=c++20 tests/test_chatmix_and_volumes.cpp src/core/channel-audio-engine.cpp src/core/routing-rules.cpp src/core/audio-session-watcher.cpp src/core/process-utils.cpp -I src -lole32 -lpsapi -o test_volumes.exe; .\test_volumes.exe`
Expected: FAIL (monitoring type logic not yet connected to sources).

- [ ] **Step 3: Implement stream volume and monitoring type updates**

In `src/core/channel-audio-engine.cpp`:
- In `setStreamVolume`: for all processes in the channel, call `obs_source_set_volume(source, std::pow(effectiveStreamVolume, 3.0f))`.
- In `setStreamMuted`: for all processes in the channel, call `obs_source_set_muted(source, effectiveStreamMuted)`.
- In `setMonitorVolume` and `setMonitorMuted`:
  - Determine `bool shouldMonitor = (effectiveMonitorVolume > 0.001f && !effectiveMonitorMuted);`
  - For all processes in the channel, call `obs_source_set_monitoring_type(source, shouldMonitor ? OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT : OBS_MONITORING_TYPE_NONE)`.
- In `applyVolumeToProcesses`: update both OBS source stream volume and OBS source monitoring type. Do NOT zero WASAPI session volume if stream volume is active.

- [ ] **Step 4: Run test to verify it passes**

Run: `& C:\msys64\ucrt64\bin\g++.exe -std=c++20 tests/test_chatmix_and_volumes.cpp src/core/channel-audio-engine.cpp src/core/routing-rules.cpp src/core/audio-session-watcher.cpp src/core/process-utils.cpp -I src -lole32 -lpsapi -o test_volumes.exe; .\test_volumes.exe`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/core/channel-audio-engine.hpp src/core/channel-audio-engine.cpp tests/test_chatmix_and_volumes.cpp
git commit -m "feat(core): connect dual-bus faders to OBS stream volume and audio monitoring"
```

---

### Task 3: UI Sync & OBS Desktop Audio Helpful Guidance

**Files:**
- Modify: `src/ui/audio-router-dock.cpp`
- Modify: `src/ui/localization-helper.hpp`

**Interfaces:**
- Consumes: `AudioRouterDock`, `I18nHelper`
- Produces: Visual tooltip and status indicator guiding users on OBS Desktop Audio separation.

- [ ] **Step 1: Add localization strings for Desktop Audio guidance**

In `src/ui/localization-helper.hpp`:
- Add `"DesktopAudioTip"` in Portuguese: `"Dica: No mixer do OBS, silencie o 'Áudio do Desktop' padrão para evitar eco, pois o Audio Router já envia os canais diretamente para a gravação/live."`
- Add English: `"Tip: In OBS mixer, mute default 'Desktop Audio' to avoid echo, as Audio Router sends channels directly to your broadcast/recording."`
- Add Spanish, French, Russian translations.

- [ ] **Step 2: Add info banner / tooltip to `AudioRouterDock`**

In `src/ui/audio-router-dock.cpp`:
- Add a subtle info icon/tooltip in the top bar with the `DesktopAudioTip`.

- [ ] **Step 3: Verify tests and compilation**

Run: `& C:\msys64\ucrt64\bin\g++.exe -std=c++20 tests/test_i18n_and_rules.cpp src/core/routing-rules.cpp -I src -o test_i18n.exe; .\test_i18n.exe`
Expected: PASS.

- [ ] **Step 4: Commit**

```bash
git add src/ui/localization-helper.hpp src/ui/audio-router-dock.cpp
git commit -m "feat(ui): add Desktop Audio tip and localization for clean OBS track separation"
```

---

### Task 4: Complete Suite Verification & Building DLL

**Files:**
- Output: `dist/obs-plugins/64bit/obs-audio-router.dll`

- [ ] **Step 1: Run all unit tests**

Execute:
1. `test_channel_audio_engine`
2. `test_chatmix_and_volumes`
3. `test_routing_rules`
4. `test_i18n_and_rules`
5. `test_wasapi_sessions`
Expected: 100% PASS.

- [ ] **Step 2: Commit all changes**

```bash
git commit -m "chore: complete dual-bus stream and monitoring implementation"
```
