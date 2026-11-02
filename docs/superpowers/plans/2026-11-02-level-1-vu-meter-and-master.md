# Level 1 Enhancements: Real-time VU Meters, Master Column & Compact Mute Buttons Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement real-time VU audio peak meters beside each fader, a dedicated Master volume column on the far left, and compact non-scalable icon mute buttons (`🔊`/`🔇`).

**Architecture:** 
- `AudioSessionWatcher`: Query `IAudioMeterInformation::GetPeakValue` from Windows WASAPI audio sessions to read peak linear amplitude (0.0 to 1.0) without CPU overhead.
- `ChannelAudioEngine`: Add `ChannelType::MASTER` support, master volume getters/setters (`setMasterMonitorVolume`, `setMasterStreamVolume`), and `getChannelPeak(ChannelType)`.
- `VuMeterWidget`: Fast, lightweight custom-painted Qt widget (4px width, 105px height) with rounded corners and a vibrant gradient (green -> yellow -> red) with exponential decay.
- `ChannelStripWidget`: Replace stretching text mute buttons with centered, fixed-size `🔊`/`🔇` icon buttons (26x20px) that never scale, and embed `VuMeterWidget` beside each fader.
- `MasterStripWidget` / Master Column: Specialized strip on the left with Master Faders (Headphones & Stream) and Master Mutes, controlling overall gains.
- `AudioRouterDock`: Run a smooth 30 FPS `QTimer` reading peak levels and updating the VU meters.

**Tech Stack:** C++20, Qt6 (Core, Gui, Widgets), Windows Core Audio WASAPI (`IAudioMeterInformation`), CMake.

---

## Task 1: Core - WASAPI Audio Peak Metering & Master Gain Logic
- [x] Step 1: Add `getProcessPeakVolume(const std::string& processName)` in `AudioSessionWatcher` using `IAudioMeterInformation`.
- [x] Step 2: In `ChannelAudioEngine`, add `ChannelType::MASTER` support, master volume getters/setters (`setMasterMonitorVolume`, `setMasterStreamVolume`), and `getChannelPeak(ChannelType)`.
- [x] Step 3: Write unit tests in `tests/test_chatmix_and_volumes.cpp` verifying master volume multipliers and channel peak aggregation.
- [x] Step 4: Run unit tests locally with `g++` to verify they pass.
- [x] Step 5: Commit changes with git.

## Task 2: UI - Compact Fixed Icon Mute Buttons & `VuMeterWidget`
- [x] Step 1: Create `src/ui/vu-meter-widget.hpp` and `src/ui/vu-meter-widget.cpp` implementing a custom Qt widget with `setLevel(float)` and gradient drawing.
- [x] Step 2: Refactor `ChannelStripWidget::createFaderColumn` to embed the `VuMeterWidget` beside the vertical `QSlider`.
- [x] Step 3: Replace full-width mute buttons with compact, fixed-size buttons (`setFixedSize(26, 20)`, centered with `Qt::AlignHCenter`) displaying `🔊` (unmuted) and `🔇` (muted in red).
- [x] Step 4: Verify compilation and layout integrity.
- [x] Step 5: Commit changes with git.

## Task 3: UI - Dedicated Master Channel Column
- [x] Step 1: Add support in `ChannelStripWidget` (or dedicated mode) for Master column: title "MASTER", icon "🎛️", no apps section, connected to Master engine volume/mute.
- [x] Step 2: In `AudioRouterDock`, add the Master column to the left of GAME, CHAT, MEDIA, SYSTEM.
- [x] Step 3: Verify Master fader correctly adjusts all active Windows audio session levels proportionally.
- [x] Step 4: Commit changes with git.

## Task 4: UI - 30 FPS Peak Refresh Timer in `AudioRouterDock`
- [x] Step 1: Add a `QTimer` in `AudioRouterDock` firing every 33ms (~30 FPS).
- [x] Step 2: On timer tick, query `getChannelPeak` for each channel and call `vuMeter->setLevel(peak)` with decay.
- [x] Step 3: Verify no CPU lag and smooth animation.
- [x] Step 4: Commit changes with git.

## Task 5: Verification, Packaging & GitHub Release `v0.3.0`
- [x] Step 1: Run all test suites (`test_i18n_and_rules`, `test_chatmix_and_volumes`).
- [x] Step 2: Commit all updates to `main`.
- [x] Step 3: Tag release `v0.3.0` and push to GitHub origin (`git push origin main; git push origin v0.3.0`).
- [x] Step 4: Confirm GitHub Actions build and provide single DLL download instructions to user.
