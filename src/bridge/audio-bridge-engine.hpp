#pragma once

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <atomic>
#include "audio-bridge-ipc.hpp"

class AudioBridgeEngine {
public:
    AudioBridgeEngine();
    ~AudioBridgeEngine();

    bool initialize();
    bool initializeTestMode();
    void runLoop();
    void stop();

    bool isRunning() const { return m_running.load(); }
    bool isStreaming() const { return m_streaming.load(); }
    bool isOBSActive() const { return m_obsActive.load(); }
    bool isReady() const { return m_ready.load(); }

    void updateState(bool obsActive);

private:
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_streaming{false};
    std::atomic<bool> m_obsActive{false};
    std::atomic<bool> m_ready{false};
    bool m_testMode{false};

    AudioBridgeIPC m_ipc;

    IMMDeviceEnumerator* m_deviceEnumerator{nullptr};
    IMMDevice* m_sourceDevice{nullptr};
    IMMDevice* m_targetDevice{nullptr};
    IAudioClient* m_captureClient{nullptr};
    IAudioCaptureClient* m_captureBufferClient{nullptr};
    IAudioClient* m_renderClient{nullptr};
    IAudioRenderClient* m_renderBufferClient{nullptr};

    bool setupAudioPipeline();
    void teardownAudioPipeline();
    void pumpAudio();
};
