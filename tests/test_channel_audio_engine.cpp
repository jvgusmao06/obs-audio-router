/**
 * @file test_channel_audio_engine.cpp
 * @brief Unit tests verifying ChannelAudioEngine volume control, channel assignment, and mute handling.
 */

#include "core/channel-audio-engine.hpp"
#include "core/process-utils.hpp"
#include "core/audio-bridge-ipc.hpp"
#include <iostream>
#include <cassert>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#endif

static int s_obsSourceReleaseCalls = 0;
static int s_obsSourceIncActiveCalls = 0;
static int s_obsSourceDecActiveCalls = 0;
static long long s_lastPriority = -1;
static std::string s_lastWindow;
static obs_source_t* s_dummySource = reinterpret_cast<obs_source_t*>(0x1234);

extern "C" {
    obs_source_t* obs_source_create(const char*, const char*, obs_data_t*, obs_data_t*) {
        return s_dummySource;
    }
    void obs_source_release(obs_source_t*) {
        s_obsSourceReleaseCalls++;
    }
    void obs_source_inc_active(obs_source_t*) {
        s_obsSourceIncActiveCalls++;
    }
    void obs_source_dec_active(obs_source_t*) {
        s_obsSourceDecActiveCalls++;
    }
    void obs_source_set_volume(obs_source_t*, float) {}
    void obs_source_set_muted(obs_source_t*, bool) {}
    void obs_source_set_monitoring_type(obs_source_t*, int) {}
    obs_data_t* obs_data_create(void) {
        return reinterpret_cast<obs_data_t*>(0x5678);
    }
    void obs_data_set_string(obs_data_t*, const char* name, const char* val) {
        if (std::strcmp(name, "window") == 0 && val) {
            s_lastWindow = val;
        }
    }
    void obs_data_set_int(obs_data_t*, const char* name, long long val) {
        if (std::strcmp(name, "priority") == 0) {
            s_lastPriority = val;
        }
    }
    void obs_data_release(obs_data_t*) {}
}

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
    // Processo vazio deve ser ignorado sem criar fontes
    engine.assignProcessToChannel("", AudioRouter::ChannelType::CHAT);

    s_lastPriority = -1;
    int incCallsBefore = s_obsSourceIncActiveCalls;
    engine.assignProcessToChannel("discord.exe", AudioRouter::ChannelType::CHAT);
    auto chatState = engine.getChannelState(AudioRouter::ChannelType::CHAT);
    assert(chatState != nullptr);
    assert(!chatState->assignedProcesses.empty());
    assert(chatState->assignedProcesses[0] == "discord.exe");
    assert(s_lastPriority == 2); // 2 = WINDOW_PRIORITY_EXE
    assert(s_lastWindow == "::discord.exe");
    assert(s_obsSourceIncActiveCalls > incCallsBefore);

    // Teste System Sounds: não deve instanciar fonte WASAPI process
    int incBeforeSystem = s_obsSourceIncActiveCalls;
    engine.assignProcessToChannel("System Sounds", AudioRouter::ChannelType::SYSTEM);
    assert(s_obsSourceIncActiveCalls == incBeforeSystem);

    // Teste de prevenção de recriação contínua (anti-thrashing):
    // Re-atribuir o mesmo app ao mesmo canal NÃO deve destruir ou recriar a fonte OBS
    int releaseCallsBefore = s_obsSourceReleaseCalls;
    incCallsBefore = s_obsSourceIncActiveCalls;
    engine.assignProcessToChannel("discord.exe", AudioRouter::ChannelType::CHAT);
    assert(s_obsSourceReleaseCalls == releaseCallsBefore);
    assert(s_obsSourceIncActiveCalls == incCallsBefore);

    // Move discord para mídia
    engine.assignProcessToChannel("discord.exe", AudioRouter::ChannelType::MEDIA);
    auto mediaState = engine.getChannelState(AudioRouter::ChannelType::MEDIA);
    assert(mediaState != nullptr);
    assert(!mediaState->assignedProcesses.empty());
    assert(mediaState->assignedProcesses[0] == "discord.exe");
    assert(chatState->assignedProcesses.empty());

    std::cout << "[TEST] 5. Testando desatribuição quando app fecha (unassignProcess)..." << std::endl;
    int decCallsBefore = s_obsSourceDecActiveCalls;
    engine.unassignProcess("discord.exe");
    assert(mediaState->assignedProcesses.empty());
    assert(s_obsSourceDecActiveCalls > decCallsBefore);

    std::cout << "[TEST] 5b. Testando ciclo de vida da fonte OBS por processo (getProcessSource)..." << std::endl;
    int prevReleaseCalls = s_obsSourceReleaseCalls;
    engine.assignProcessToChannel("spotify.exe", AudioRouter::ChannelType::MEDIA);
    assert(engine.getProcessSource("spotify.exe") != nullptr);

    engine.unassignProcess("spotify.exe");
    assert(engine.getProcessSource("spotify.exe") == nullptr);
    assert(s_obsSourceReleaseCalls > prevReleaseCalls);

    std::cout << "[TEST] 6. Testando verificação de processo ativo (ProcessUtils::isProcessRunning)..." << std::endl;
    assert(AudioRouter::ProcessUtils::isProcessRunning(0) == true); // System sounds
#ifdef _WIN32
    assert(AudioRouter::ProcessUtils::isProcessRunning(GetCurrentProcessId()) == true);
    assert(AudioRouter::ProcessUtils::isProcessRunning(0xFFFFFFFE) == false); // PID inexistente
#endif

    std::cout << "[TEST] 7. Testando modo de Roteamento Virtual e IPC Bridge..." << std::endl;
    AudioBridgeIPC testIPC;
    testIPC.setActive(false);
    assert(!testIPC.isOBSActive());

    engine.setVirtualRoutingEnabled(true);
    assert(engine.isVirtualRoutingEnabled() == true);
    assert(testIPC.isOBSActive() == true);

    engine.setVirtualRoutingEnabled(false);
    assert(engine.isVirtualRoutingEnabled() == false);
    assert(testIPC.isOBSActive() == false);

    std::cout << "[SUCCESS] Todos os testes do motor de áudio passaram com êxito!" << std::endl;
    return 0;
}
