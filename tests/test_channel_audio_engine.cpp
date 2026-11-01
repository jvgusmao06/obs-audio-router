#include "core/channel-audio-engine.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[TEST] 1. Inicializando ChannelAudioEngine..." << std::endl;
    auto& engine = AudioRouter::ChannelAudioEngine::instance();

    std::cout << "[TEST] 2. Testando controle de volumes (Stream e Monitor)..." << std::endl;
    engine.setStreamVolume(AudioRouter::ChannelType::GAME, 0.75f);
    assert(engine.getStreamVolume(AudioRouter::ChannelType::GAME) == 0.75f);

    engine.setMonitorVolume(AudioRouter::ChannelType::GAME, 0.90f);
    assert(engine.getMonitorVolume(AudioRouter::ChannelType::GAME) == 0.90f);

    std::cout << "[TEST] 3. Testando controle de Mudo..." << std::endl;
    engine.setMuted(AudioRouter::ChannelType::CHAT, true);
    assert(engine.isMuted(AudioRouter::ChannelType::CHAT) == true);
    engine.setMuted(AudioRouter::ChannelType::CHAT, false);
    assert(engine.isMuted(AudioRouter::ChannelType::CHAT) == false);

    std::cout << "[TEST] 4. Testando atribuição e remanejamento de processos..." << std::endl;
    engine.assignProcessToChannel("discord.exe", AudioRouter::ChannelType::CHAT);
    auto chatState = engine.getChannelState(AudioRouter::ChannelType::CHAT);
    assert(chatState != nullptr);
    assert(!chatState->assignedProcesses.empty());
    assert(chatState->assignedProcesses[0] == "discord.exe");

    // Move discord para mídia
    engine.assignProcessToChannel("discord.exe", AudioRouter::ChannelType::MEDIA);
    auto mediaState = engine.getChannelState(AudioRouter::ChannelType::MEDIA);
    assert(mediaState != nullptr);
    assert(!mediaState->assignedProcesses.empty());
    assert(mediaState->assignedProcesses[0] == "discord.exe");
    assert(chatState->assignedProcesses.empty());

    std::cout << "[SUCCESS] Todos os testes do motor de áudio passaram com êxito!" << std::endl;
    return 0;
}

