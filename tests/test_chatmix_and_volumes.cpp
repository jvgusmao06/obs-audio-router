/**
 * @file test_chatmix_and_volumes.cpp
 * @brief Unit tests verifying ChatMix attenuation math, independent stream/monitor mutes,
 *        and Stream Monitoring mode state transitions.
 */

#include "core/channel-audio-engine.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

extern "C" {
    void obs_source_release(obs_source_t*) {}
    void obs_source_set_volume(obs_source_t*, float) {}
    void obs_source_set_muted(obs_source_t*, bool) {}
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

    std::cout << "[SUCCESS] Todos os testes de ChatMix e volumes passaram!" << std::endl;
    return 0;
}
