#include "audio-bridge-engine.hpp"
#include <iostream>
#include <thread>
#include <chrono>

AudioBridgeEngine::AudioBridgeEngine() {
}

AudioBridgeEngine::~AudioBridgeEngine() {
    stop();
    teardownAudioPipeline();
}

bool AudioBridgeEngine::initializeTestMode() {
    m_testMode = true;
    m_running = true;
    m_ready = true;
    return true;
}

bool AudioBridgeEngine::initialize() {
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return false;
    }

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&m_deviceEnumerator);
    if (FAILED(hr)) {
        return false;
    }

    m_running = true;
    m_ready = true;
    return true;
}

void AudioBridgeEngine::updateState(bool obsActive) {
    m_obsActive = obsActive;
    if (obsActive) {
        // When OBS is active, the bridge must pause audio passthrough
        m_streaming = false;
    } else {
        // When OBS is not active, the bridge can stream audio
        if (m_ready && !m_testMode) {
            m_streaming = true;
        }
    }
}

bool AudioBridgeEngine::setupAudioPipeline() {
    if (!m_deviceEnumerator) return false;

    teardownAudioPipeline();

    // Default console/multimedia playback device (which receives Windows audio)
    HRESULT hr = m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_sourceDevice);
    if (FAILED(hr) || !m_sourceDevice) return false;

    // Capture client in loopback mode
    hr = m_sourceDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&m_captureClient);
    if (FAILED(hr) || !m_captureClient) return false;

    WAVEFORMATEX* pwfx = nullptr;
    hr = m_captureClient->GetMixFormat(&pwfx);
    if (FAILED(hr) || !pwfx) return false;

    REFERENCE_TIME hnsBufferDuration = 100000; // 10ms
    hr = m_captureClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        hnsBufferDuration,
        0,
        pwfx,
        NULL
    );

    if (SUCCEEDED(hr)) {
        hr = m_captureClient->GetService(__uuidof(IAudioCaptureClient), (void**)&m_captureBufferClient);
    }

    if (pwfx) {
        CoTaskMemFree(pwfx);
    }

    if (FAILED(hr)) {
        teardownAudioPipeline();
        return false;
    }

    return true;
}

void AudioBridgeEngine::teardownAudioPipeline() {
    if (m_captureClient) {
        m_captureClient->Stop();
    }
    if (m_renderClient) {
        m_renderClient->Stop();
    }

    if (m_captureBufferClient) {
        m_captureBufferClient->Release();
        m_captureBufferClient = nullptr;
    }
    if (m_captureClient) {
        m_captureClient->Release();
        m_captureClient = nullptr;
    }
    if (m_renderBufferClient) {
        m_renderBufferClient->Release();
        m_renderBufferClient = nullptr;
    }
    if (m_renderClient) {
        m_renderClient->Release();
        m_renderClient = nullptr;
    }
    if (m_sourceDevice) {
        m_sourceDevice->Release();
        m_sourceDevice = nullptr;
    }
    if (m_targetDevice) {
        m_targetDevice->Release();
        m_targetDevice = nullptr;
    }
}

void AudioBridgeEngine::stop() {
    m_running = false;
    m_streaming = false;
}

void AudioBridgeEngine::pumpAudio() {
    if (!m_captureBufferClient) return;

    UINT32 packetLength = 0;
    HRESULT hr = m_captureBufferClient->GetNextPacketSize(&packetLength);
    while (SUCCEEDED(hr) && packetLength > 0) {
        BYTE* pData = nullptr;
        UINT32 numFramesAvailable = 0;
        DWORD flags = 0;

        hr = m_captureBufferClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
        if (SUCCEEDED(hr)) {
            // In full hardware passthrough, samples are written to m_renderBufferClient here
            m_captureBufferClient->ReleaseBuffer(numFramesAvailable);
        }
        hr = m_captureBufferClient->GetNextPacketSize(&packetLength);
    }
}

void AudioBridgeEngine::runLoop() {
    if (m_testMode) return;

    while (m_running) {
        bool obsActive = m_ipc.isOBSActive();
        updateState(obsActive);

        if (obsActive) {
            // OBS is handling audio; sleep and release pipeline to preserve resources
            if (m_captureClient) {
                teardownAudioPipeline();
            }
            // Wait up to 500ms or until signaled
            HANDLE hEvent = m_ipc.getEventHandle();
            if (hEvent) {
                WaitForSingleObject(hEvent, 500);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
        } else {
            // OBS is closed; ensure audio passthrough is running
            if (!m_captureClient) {
                if (!setupAudioPipeline()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }
                m_captureClient->Start();
                m_streaming = true;
            }

            pumpAudio();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    teardownAudioPipeline();
}
