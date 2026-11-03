/**
 * @file test_chatmix_and_volumes.cpp
 * @brief Unit tests verifying ChatMix attenuation math, independent stream/monitor mutes,
 *        Stream Monitoring mode state transitions, and Master channel scaling.
 */

#include "core/channel-audio-engine.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

enum obs_monitoring_type {
    OBS_MONITORING_TYPE_NONE = 0,
    OBS_MONITORING_TYPE_MONITOR_ONLY = 1,
    OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT = 2
};

static float s_lastSetVolume = -1.0f;
static bool s_lastSetMuted = false;
static int s_lastMonitoringType = -1;
static obs_source_t* s_mockSource = reinterpret_cast<obs_source_t*>(0x1234);

extern "C" {
    obs_source_t* obs_source_create(const char*, const char*, obs_data_t*, obs_data_t*) {
        return s_mockSource;
    }
    void obs_source_release(obs_source_t*) {}
    void obs_source_set_volume(obs_source_t*, float volume) {
        s_lastSetVolume = volume;
    }
    void obs_source_set_muted(obs_source_t*, bool muted) {
        s_lastSetMuted = muted;
    }
    void obs_source_set_monitoring_type(obs_source_t*, enum obs_monitoring_type type) {
        s_lastMonitoringType = static_cast<int>(type);
    }
    obs_data_t* obs_data_create(void) {
        return reinterpret_cast<obs_data_t*>(0x5678);
    }
    void obs_data_set_string(obs_data_t*, const char*, const char*) {}
    void obs_data_release(obs_data_t*) {}
}

static bool approxEqual(float a, float b, float eps = 0.001f) {
    return std::fabs(a - b) < eps;
}

int main() {
    std::cout << "[TEST] 1. ChatMix no centro (0.0): ambos a 100%..." << std::endl;
    auto [game1, chat1] = AudioRouter::ChannelAudioEngine::calculateChatMixGains(0.0f, 1.0f, 1.0f);
    assert(approxEqual(game1, 1.0f));
    assert(approxEqual(chat1, 1.0f));

    std::cout << "[TEST] 2. ChatMix em -0.5 (favor Game): Game 100%, Chat 50%..." << std::endl;
    auto [game2, chat2] = AudioRouter::ChannelAudioEngine::calculateChatMixGains(-0.5f, 1.0f, 1.0f);
    assert(approxEqual(game2, 1.0f));
    assert(approxEqual(chat2, 0.5f));

    std::cout << "[TEST] 3. ChatMix em -1.0 (full Game): Game 100%, Chat 0%..." << std::endl;
    auto [game3, chat3] = AudioRouter::ChannelAudioEngine::calculateChatMixGains(-1.0f, 1.0f, 1.0f);
    assert(approxEqual(game3, 1.0f));
    assert(approxEqual(chat3, 0.0f));

    std::cout << "[TEST] 4. ChatMix em +0.5 (favor Chat): Game 50%, Chat 100%..." << std::endl;
    auto [game4, chat4] = AudioRouter::ChannelAudioEngine::calculateChatMixGains(0.5f, 1.0f, 1.0f);
    assert(approxEqual(game4, 0.5f));
    assert(approxEqual(chat4, 1.0f));

    std::cout << "[TEST] 5. ChatMix em +1.0 (full Chat): Game 0%, Chat 100%..." << std::endl;
    auto [game5, chat5] = AudioRouter::ChannelAudioEngine::calculateChatMixGains(1.0f, 1.0f, 1.0f);
    assert(approxEqual(game5, 0.0f));
    assert(approxEqual(chat5, 1.0f));

    std::cout << "[TEST] 6. ChatMix com volume base de 80% e 60%..." << std::endl;
    auto [game6, chat6] = AudioRouter::ChannelAudioEngine::calculateChatMixGains(-0.5f, 0.8f, 0.6f);
    assert(approxEqual(game6, 0.8f));
    assert(approxEqual(chat6, 0.3f)); // 0.6 * 0.5 = 0.3

    std::cout << "[TEST] 7. Clamping de valores fora da faixa (-2.0 e +2.0)..." << std::endl;
    auto [game7, chat7] = AudioRouter::ChannelAudioEngine::calculateChatMixGains(-2.0f, 1.0f, 1.0f);
    assert(approxEqual(game7, 1.0f));
    assert(approxEqual(chat7, 0.0f));

    std::cout << "[TEST] 8. Testando mutes independentes (Stream vs Monitor)..." << std::endl;
    auto& engine = AudioRouter::ChannelAudioEngine::instance();
    engine.setStreamMuted(AudioRouter::ChannelType::GAME, true);
    engine.setMonitorMuted(AudioRouter::ChannelType::GAME, false);
    assert(engine.isStreamMuted(AudioRouter::ChannelType::GAME) == true);
    assert(engine.isMonitorMuted(AudioRouter::ChannelType::GAME) == false);

    engine.setMonitorMuted(AudioRouter::ChannelType::GAME, true);
    assert(engine.isMonitorMuted(AudioRouter::ChannelType::GAME) == true);
    engine.setStreamMuted(AudioRouter::ChannelType::GAME, false);
    assert(engine.isStreamMuted(AudioRouter::ChannelType::GAME) == false);
    assert(engine.isMonitorMuted(AudioRouter::ChannelType::GAME) == true);

    std::cout << "[TEST] 9. Testando modo 'Ouvir Stream' (Stream Monitoring)..." << std::endl;
    engine.setStreamVolume(AudioRouter::ChannelType::GAME, 0.4f);
    engine.setMonitorVolume(AudioRouter::ChannelType::GAME, 0.9f);
    assert(engine.isMonitoringStream() == false);
    engine.setMonitoringStream(true);
    assert(engine.isMonitoringStream() == true);
    engine.setMonitoringStream(false);
    assert(engine.isMonitoringStream() == false);

    std::cout << "[TEST] 10. Testando canal MASTER e escala global..." << std::endl;
    engine.setMonitorVolume(AudioRouter::ChannelType::MASTER, 0.8f);
    assert(approxEqual(engine.getMonitorVolume(AudioRouter::ChannelType::MASTER), 0.8f));
    engine.setStreamVolume(AudioRouter::ChannelType::MASTER, 0.65f);
    assert(approxEqual(engine.getStreamVolume(AudioRouter::ChannelType::MASTER), 0.65f));

    engine.setStreamMuted(AudioRouter::ChannelType::MASTER, true);
    assert(engine.isStreamMuted(AudioRouter::ChannelType::MASTER) == true);
    assert(engine.isMonitorMuted(AudioRouter::ChannelType::MASTER) == false);
    engine.setStreamMuted(AudioRouter::ChannelType::MASTER, false);
    assert(engine.isStreamMuted(AudioRouter::ChannelType::MASTER) == false);

    std::cout << "[TEST] 11. Testando calculo de pico de audio (getChannelPeak)..." << std::endl;
    float peak = engine.getChannelPeak(AudioRouter::ChannelType::GAME);
    assert(peak >= 0.0f && peak <= 1.0f);
    float masterPeak = engine.getChannelPeak(AudioRouter::ChannelType::MASTER);
    assert(masterPeak >= 0.0f && masterPeak <= 1.0f);

    std::cout << "[TEST] 12. Testando Dual-Bus Fader Control (Stream Volume vs Monitor Type)..." << std::endl;
    engine.setStreamVolume(AudioRouter::ChannelType::MASTER, 1.0f);
    engine.setMonitorVolume(AudioRouter::ChannelType::MASTER, 1.0f);
    engine.setStreamMuted(AudioRouter::ChannelType::MASTER, false);
    engine.setMonitorMuted(AudioRouter::ChannelType::MASTER, false);

    engine.assignProcessToChannel("spotify.exe", AudioRouter::ChannelType::MEDIA);

    // Reset trackers
    s_lastMonitoringType = -1;
    s_lastSetVolume = -1.0f;

    // Monitor volume = 0.0f, Stream volume = 1.0f
    engine.setMonitorVolume(AudioRouter::ChannelType::MEDIA, 0.0f);
    engine.setStreamVolume(AudioRouter::ChannelType::MEDIA, 1.0f);

    assert(s_lastMonitoringType == OBS_MONITORING_TYPE_NONE);
    assert(engine.getStreamVolume(AudioRouter::ChannelType::MEDIA) == 1.0f);
    assert(s_lastSetVolume > 0.0f);
    assert(approxEqual(s_lastSetVolume, 1.0f));

    // Monitor volume = 0.8f -> OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT
    engine.setMonitorVolume(AudioRouter::ChannelType::MEDIA, 0.8f);
    assert(s_lastMonitoringType == OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT);

    engine.unassignProcess("spotify.exe");

    std::cout << "[SUCCESS] Todos os testes de ChatMix, Master e volumes passaram!" << std::endl;
    return 0;
}
