#include "channel-audio-engine.hpp"
#include "audio-session-watcher.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

#if defined(__has_include) && __has_include(<obs.h>)
    #include <obs.h>
#else
    extern "C" {
        typedef struct obs_source obs_source_t;
        typedef struct obs_data obs_data_t;
        enum obs_monitoring_type {
            OBS_MONITORING_TYPE_NONE,
            OBS_MONITORING_TYPE_MONITOR_ONLY,
            OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT
        };
        extern obs_source_t* obs_source_create(const char *id, const char *name, obs_data_t *settings, obs_data_t *hotkey_data);
        extern void obs_source_release(obs_source_t *source);
        extern void obs_source_set_volume(obs_source_t *source, float volume);
        extern float obs_source_get_volume(const obs_source_t *source);
        extern void obs_source_set_muted(obs_source_t *source, bool muted);
        extern bool obs_source_muted(const obs_source_t *source);
        extern void obs_source_set_monitoring_type(obs_source_t *source, enum obs_monitoring_type type);
    }
#endif

namespace AudioRouter {

ChannelAudioEngine& ChannelAudioEngine::instance() {
    static ChannelAudioEngine s_instance;
    return s_instance;
}

ChannelAudioEngine::ChannelAudioEngine() {
    initializeChannels();
}

ChannelAudioEngine::~ChannelAudioEngine() {
    shutdownChannels();
}

void ChannelAudioEngine::initializeChannels() {
    m_channels[ChannelType::GAME]   = { ChannelType::GAME,   "Audio Router - Game",   0.85f, 1.0f, false, false, {}, nullptr };
    m_channels[ChannelType::CHAT]   = { ChannelType::CHAT,   "Audio Router - Chat",   1.00f, 1.0f, false, false, {}, nullptr };
    m_channels[ChannelType::MEDIA]  = { ChannelType::MEDIA,  "Audio Router - Media",  0.50f, 0.8f, false, false, {}, nullptr };
    m_channels[ChannelType::SYSTEM] = { ChannelType::SYSTEM, "Audio Router - System", 0.70f, 0.7f, false, false, {}, nullptr };
}

void ChannelAudioEngine::shutdownChannels() {
    for (auto& [type, state] : m_channels) {
        if (state.obsSource) {
            obs_source_release(state.obsSource);
            state.obsSource = nullptr;
        }
    }
}

void ChannelAudioEngine::setStreamVolume(ChannelType channel, float volumeNormalized) {
    volumeNormalized = std::clamp(volumeNormalized, 0.0f, 1.0f);
    m_channels[channel].streamVolume = volumeNormalized;

    // Atualiza volume na fonte nativa do OBS (converte fader linear em ganho cúbico OBS)
    if (m_channels[channel].obsSource) {
        float obsVolume = std::pow(volumeNormalized, 3.0f);
        obs_source_set_volume(m_channels[channel].obsSource, obsVolume);
    }
}

float ChannelAudioEngine::getStreamVolume(ChannelType channel) const {
    auto it = m_channels.find(channel);
    return (it != m_channels.end()) ? it->second.streamVolume : 1.0f;
}

void ChannelAudioEngine::setMonitorVolume(ChannelType channel, float volumeNormalized) {
    volumeNormalized = std::clamp(volumeNormalized, 0.0f, 1.0f);
    m_channels[channel].monitorVolume = volumeNormalized;
    applyVolumeToProcesses(channel);
}

float ChannelAudioEngine::getMonitorVolume(ChannelType channel) const {
    auto it = m_channels.find(channel);
    return (it != m_channels.end()) ? it->second.monitorVolume : 1.0f;
}

float ChannelAudioEngine::getEffectiveMonitorVolume(ChannelType channel) const {
    if (channel == ChannelType::GAME) {
        return calculateChatMixGains(m_chatMix, getMonitorVolume(ChannelType::GAME), getMonitorVolume(ChannelType::CHAT)).first;
    } else if (channel == ChannelType::CHAT) {
        return calculateChatMixGains(m_chatMix, getMonitorVolume(ChannelType::GAME), getMonitorVolume(ChannelType::CHAT)).second;
    }
    return getMonitorVolume(channel);
}

void ChannelAudioEngine::setChatMix(float factor) {
    m_chatMix = std::clamp(factor, -1.0f, 1.0f);
    applyVolumeToProcesses(ChannelType::GAME);
    applyVolumeToProcesses(ChannelType::CHAT);
}

float ChannelAudioEngine::getChatMix() const {
    return m_chatMix;
}

std::pair<float, float> ChannelAudioEngine::calculateChatMixGains(float chatMixFactor, float gameVol, float chatVol) {
    float clampedMix = std::clamp(chatMixFactor, -1.0f, 1.0f);
    gameVol = std::clamp(gameVol, 0.0f, 1.0f);
    chatVol = std::clamp(chatVol, 0.0f, 1.0f);

    float gameMultiplier = 1.0f;
    float chatMultiplier = 1.0f;

    if (clampedMix < 0.0f) {
        // Favor Game (-1.0 a 0.0): Game em 1.0, Chat atenuado
        chatMultiplier = 1.0f + clampedMix;
    } else if (clampedMix > 0.0f) {
        // Favor Chat (0.0 a 1.0): Chat em 1.0, Game atenuado
        gameMultiplier = 1.0f - clampedMix;
    }

    return { gameVol * gameMultiplier, chatVol * chatMultiplier };
}

void ChannelAudioEngine::setSessionWatcher(AudioSessionWatcher *watcher) {
    m_sessionWatcher = watcher;
    if (m_sessionWatcher) {
        for (auto type : {ChannelType::GAME, ChannelType::CHAT, ChannelType::MEDIA, ChannelType::SYSTEM}) {
            applyVolumeToProcesses(type);
            applyMuteToProcesses(type);
        }
    }
}

AudioSessionWatcher* ChannelAudioEngine::getSessionWatcher() const {
    return m_sessionWatcher;
}

void ChannelAudioEngine::applyVolumeToProcesses(ChannelType channel) {
    if (!m_sessionWatcher) return;
    auto it = m_channels.find(channel);
    if (it == m_channels.end()) return;

    float effectiveVol = getEffectiveMonitorVolume(channel);
    for (const auto& proc : it->second.assignedProcesses) {
        m_sessionWatcher->setProcessVolume(proc, effectiveVol);
    }
}

void ChannelAudioEngine::applyMuteToProcesses(ChannelType channel) {
    if (!m_sessionWatcher) return;
    auto it = m_channels.find(channel);
    if (it == m_channels.end()) return;

    bool muted = it->second.isMonitorMuted;
    for (const auto& proc : it->second.assignedProcesses) {
        m_sessionWatcher->setProcessMute(proc, muted);
    }
}

void ChannelAudioEngine::setStreamMuted(ChannelType channel, bool muted) {
    m_channels[channel].isStreamMuted = muted;
    if (m_channels[channel].obsSource) {
        obs_source_set_muted(m_channels[channel].obsSource, muted);
    }
}

bool ChannelAudioEngine::isStreamMuted(ChannelType channel) const {
    auto it = m_channels.find(channel);
    return (it != m_channels.end()) ? it->second.isStreamMuted : false;
}

void ChannelAudioEngine::setMonitorMuted(ChannelType channel, bool muted) {
    m_channels[channel].isMonitorMuted = muted;
    applyMuteToProcesses(channel);
}

bool ChannelAudioEngine::isMonitorMuted(ChannelType channel) const {
    auto it = m_channels.find(channel);
    return (it != m_channels.end()) ? it->second.isMonitorMuted : false;
}

void ChannelAudioEngine::assignProcessToChannel(const std::string& processName, ChannelType channel) {
    unassignProcess(processName);

    m_processChannelMap[processName] = channel;
    auto& list = m_channels[channel].assignedProcesses;
    if (std::find(list.begin(), list.end(), processName) == list.end()) {
        list.push_back(processName);
    }

    syncObsSourcesForChannel(channel);
    applyVolumeToProcesses(channel);
    applyMuteToProcesses(channel);
}

void ChannelAudioEngine::unassignProcess(const std::string& processName) {
    auto it = m_processChannelMap.find(processName);
    if (it != m_processChannelMap.end()) {
        ChannelType oldChannel = it->second;
        auto& list = m_channels[oldChannel].assignedProcesses;
        list.erase(std::remove(list.begin(), list.end(), processName), list.end());
        m_processChannelMap.erase(it);
        syncObsSourcesForChannel(oldChannel);
    }
}

const ChannelAudioState* ChannelAudioEngine::getChannelState(ChannelType channel) const {
    auto it = m_channels.find(channel);
    return (it != m_channels.end()) ? &it->second : nullptr;
}

void ChannelAudioEngine::syncObsSourcesForChannel(ChannelType channel) {
    // Sincroniza fontes WASAPI Application Audio Capture internas associadas ao canal
    // No OBS Studio 29+, a fonte nativa "wasapi_process_output_capture" é instanciada para cada aplicativo
}

} // namespace AudioRouter
