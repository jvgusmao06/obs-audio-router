/**
 * @file audio-session-watcher.cpp
 * @brief Implementation of AudioSessionWatcher using Windows Core Audio APIs (WASAPI)
 *        to enumerate audio sessions, listen for new audio clients, and control volume.
 */

#include "audio-session-watcher.hpp"
#include "process-utils.hpp"

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace AudioRouter {

class SessionNotificationHandler : public IAudioSessionNotification {
public:
    SessionNotificationHandler(std::function<void(IAudioSessionControl*)> onCreated)
        : callback(onCreated), refCount(1) {}

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) override {
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IAudioSessionNotification)) {
            *ppv = static_cast<IAudioSessionNotification*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&refCount);
    }

    STDMETHODIMP_(ULONG) Release() override {
        ULONG count = InterlockedDecrement(&refCount);
        if (count == 0) {
            delete this;
        }
        return count;
    }

    // IAudioSessionNotification
    STDMETHODIMP OnSessionCreated(IAudioSessionControl *NewSession) override {
        if (callback && NewSession) {
            callback(NewSession);
        }
        return S_OK;
    }

private:
    std::function<void(IAudioSessionControl*)> callback;
    long refCount;
};

class AudioSessionWatcher::Impl {
public:
    Impl() = default;
    ~Impl() {
        stop();
    }

    bool start() {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        coInitialized = SUCCEEDED(hr);

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                              __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(deviceEnumerator.GetAddressOf()));
        if (FAILED(hr)) return false;

        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, device.GetAddressOf());
        if (FAILED(hr)) return false;

        hr = device->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr,
                              reinterpret_cast<void**>(sessionManager.GetAddressOf()));
        if (FAILED(hr)) return false;

        notificationHandler = new SessionNotificationHandler([this](IAudioSessionControl *session) {
            handleNewSession(session);
        });

        sessionManager->RegisterSessionNotification(notificationHandler.Get());
        return true;
    }

    void stop() {
        if (sessionManager && notificationHandler) {
            sessionManager->UnregisterSessionNotification(notificationHandler.Get());
            notificationHandler.Reset();
        }
        sessionManager.Reset();
        device.Reset();
        deviceEnumerator.Reset();

        if (coInitialized) {
            CoUninitialize();
            coInitialized = false;
        }
    }

    std::vector<AudioSessionInfo> getActiveSessions() {
        std::vector<AudioSessionInfo> sessions;
        if (!sessionManager) return sessions;

        ComPtr<IAudioSessionEnumerator> sessionEnum;
        HRESULT hr = sessionManager->GetSessionEnumerator(sessionEnum.GetAddressOf());
        if (FAILED(hr)) return sessions;

        int count = 0;
        sessionEnum->GetCount(&count);

        for (int i = 0; i < count; ++i) {
            ComPtr<IAudioSessionControl> control;
            if (SUCCEEDED(sessionEnum->GetSession(i, control.GetAddressOf()))) {
                AudioSessionInfo info = parseSessionControl(control.Get());
                if (!info.processName.empty() || info.isSystemSound) {
                    sessions.push_back(info);
                }
            }
        }
        return sessions;
    }

    bool setProcessVolume(const std::string& processName, float volumeNormalized) {
        if (!sessionManager) return false;
        ComPtr<IAudioSessionEnumerator> sessionEnum;
        if (FAILED(sessionManager->GetSessionEnumerator(sessionEnum.GetAddressOf()))) return false;

        int count = 0;
        sessionEnum->GetCount(&count);
        bool found = false;

        for (int i = 0; i < count; ++i) {
            ComPtr<IAudioSessionControl> control;
            if (SUCCEEDED(sessionEnum->GetSession(i, control.GetAddressOf()))) {
                AudioSessionInfo info = parseSessionControl(control.Get());
                bool match = false;
                if (processName == "System Sounds" && info.isSystemSound) {
                    match = true;
                } else if (_stricmp(info.processName.c_str(), processName.c_str()) == 0) {
                    match = true;
                }

                if (match) {
                    ComPtr<ISimpleAudioVolume> simpleVol;
                    if (SUCCEEDED(control.As(&simpleVol))) {
                        float vol = (volumeNormalized < 0.0f) ? 0.0f : (volumeNormalized > 1.0f ? 1.0f : volumeNormalized);
                        simpleVol->SetMasterVolume(vol, nullptr);
                        found = true;
                    }
                }
            }
        }
        return found;
    }

    bool setProcessMute(const std::string& processName, bool muted) {
        if (!sessionManager) return false;
        ComPtr<IAudioSessionEnumerator> sessionEnum;
        if (FAILED(sessionManager->GetSessionEnumerator(sessionEnum.GetAddressOf()))) return false;

        int count = 0;
        sessionEnum->GetCount(&count);
        bool found = false;

        for (int i = 0; i < count; ++i) {
            ComPtr<IAudioSessionControl> control;
            if (SUCCEEDED(sessionEnum->GetSession(i, control.GetAddressOf()))) {
                AudioSessionInfo info = parseSessionControl(control.Get());
                bool match = false;
                if (processName == "System Sounds" && info.isSystemSound) {
                    match = true;
                } else if (_stricmp(info.processName.c_str(), processName.c_str()) == 0) {
                    match = true;
                }

                if (match) {
                    ComPtr<ISimpleAudioVolume> simpleVol;
                    if (SUCCEEDED(control.As(&simpleVol))) {
                        simpleVol->SetMute(muted ? TRUE : FALSE, nullptr);
                        found = true;
                    }
                }
            }
        }
        return found;
    }

    void setOnSessionCreated(SessionCallback cb) {
        onCreatedCallback = cb;
    }

private:
    AudioSessionInfo parseSessionControl(IAudioSessionControl *control) {
        AudioSessionInfo info;
        if (!control) return info;

        ComPtr<IAudioSessionControl2> control2;
        if (SUCCEEDED(control->QueryInterface(__uuidof(IAudioSessionControl2), reinterpret_cast<void**>(control2.GetAddressOf())))) {
            DWORD pid = 0;
            control2->GetProcessId(&pid);
            info.processId = static_cast<uint32_t>(pid);

            if (control2->IsSystemSoundsSession() == S_OK) {
                info.isSystemSound = true;
                info.processName = "System Sounds";
            } else if (pid != 0) {
                info.processName = ProcessUtils::getProcessName(pid);
            }

            AudioSessionState state;
            if (SUCCEEDED(control->GetState(&state))) {
                info.isActive = (state == AudioSessionStateActive);
            }
        }
        return info;
    }

    void handleNewSession(IAudioSessionControl *session) {
        if (!onCreatedCallback) return;
        AudioSessionInfo info = parseSessionControl(session);
        if (!info.processName.empty()) {
            onCreatedCallback(info);
        }
    }

    bool coInitialized = false;
    ComPtr<IMMDeviceEnumerator> deviceEnumerator;
    ComPtr<IMMDevice> device;
    ComPtr<IAudioSessionManager2> sessionManager;
    ComPtr<SessionNotificationHandler> notificationHandler;
    SessionCallback onCreatedCallback;
};

} // namespace AudioRouter

#else // Non-Windows Stub

namespace AudioRouter {
class AudioSessionWatcher::Impl {
public:
    bool start() { return false; }
    void stop() {}
    std::vector<AudioSessionInfo> getActiveSessions() { return {}; }
    void setOnSessionCreated(SessionCallback) {}
    bool setProcessVolume(const std::string&, float) { return true; }
    bool setProcessMute(const std::string&, bool) { return true; }
};
}

#endif // _WIN32

namespace AudioRouter {

AudioSessionWatcher::AudioSessionWatcher()
    : pImpl(std::make_unique<Impl>()) {}

AudioSessionWatcher::~AudioSessionWatcher() = default;

bool AudioSessionWatcher::start() {
    return pImpl->start();
}

void AudioSessionWatcher::stop() {
    pImpl->stop();
}

std::vector<AudioSessionInfo> AudioSessionWatcher::getActiveSessions() {
    return pImpl->getActiveSessions();
}

void AudioSessionWatcher::setOnSessionCreated(SessionCallback callback) {
    pImpl->setOnSessionCreated(callback);
}

bool AudioSessionWatcher::setProcessVolume(const std::string& processName, float volumeNormalized) {
    return pImpl->setProcessVolume(processName, volumeNormalized);
}

bool AudioSessionWatcher::setProcessMute(const std::string& processName, bool muted) {
    return pImpl->setProcessMute(processName, muted);
}

} // namespace AudioRouter

