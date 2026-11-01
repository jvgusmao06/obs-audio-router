#include "core/audio-session-watcher.hpp"
#include "core/routing-rules.hpp"
#include <iostream>

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Teste WASAPI Audio Sessions (Windows)  " << std::endl;
    std::cout << "========================================" << std::endl;

    AudioRouter::AudioSessionWatcher watcher;
    AudioRouter::RoutingRules rules;

    if (!watcher.start()) {
        std::cerr << "[FALHA] Não foi possível inicializar WASAPI ou dispositivo de áudio." << std::endl;
        return 1;
    }

    std::cout << "[OK] WASAPI Session Watcher inicializado com sucesso!" << std::endl;

    auto sessions = watcher.getActiveSessions();
    std::cout << "\nSessões de áudio detectadas no momento: " << sessions.size() << std::endl;

    for (const auto& s : sessions) {
        auto channel = rules.resolveChannel(s.processName);
        std::cout << "  - Processo: " << (s.processName.empty() ? "(Desconhecido)" : s.processName)
                  << " (PID: " << s.processId << ")"
                  << " | Estado: " << (s.isActive ? "Tocando Áudio" : "Inativo")
                  << " -> Canal Alvo: [ " << AudioRouter::channelToString(channel) << " ]"
                  << std::endl;
    }

    std::cout << "\n[SUCESSO] Teste de enumeração e mapeamento concluído com êxito!" << std::endl;
    return 0;
}

