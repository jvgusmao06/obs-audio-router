#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <memory>

namespace AudioRouter {

struct AudioSessionInfo {
    uint32_t processId = 0;
    std::string processName;
    bool isSystemSound = false;
    bool isActive = false;
};

using SessionCallback = std::function<void(const AudioSessionInfo&)>;

class AudioSessionWatcher {
public:
    AudioSessionWatcher();
    ~AudioSessionWatcher();

    // Inicia a escuta de eventos WASAPI
    bool start();
    void stop();

    // Retorna a lista de sessões de áudio que existem no momento
    std::vector<AudioSessionInfo> getActiveSessions();

    // Registra callback chamado quando uma nova aplicação começa a emitir som
    void setOnSessionCreated(SessionCallback callback);

    // Ajusta o volume e o mudo da sessão de áudio do processo no Windows via WASAPI (ISimpleAudioVolume)
    bool setProcessVolume(const std::string& processName, float volumeNormalized);
    bool setProcessMute(const std::string& processName, bool muted);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace AudioRouter

