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

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace AudioRouter

