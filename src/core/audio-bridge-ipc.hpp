#pragma once

#include <windows.h>

class AudioBridgeIPC {
public:
    static constexpr const wchar_t* EVENT_NAME_GLOBAL = L"Global\\OBSAudioRouterActiveEvent";
    static constexpr const wchar_t* EVENT_NAME_LOCAL  = L"Local\\OBSAudioRouterActiveEvent";

    AudioBridgeIPC();
    ~AudioBridgeIPC();

    AudioBridgeIPC(const AudioBridgeIPC&) = delete;
    AudioBridgeIPC& operator=(const AudioBridgeIPC&) = delete;
    AudioBridgeIPC(AudioBridgeIPC&& other) noexcept;
    AudioBridgeIPC& operator=(AudioBridgeIPC&& other) noexcept;

    bool setActive(bool active);
    bool isOBSActive() const;
    HANDLE getEventHandle() const;

private:
    HANDLE m_eventHandle{NULL};
    bool ensureEventHandle();
};
