#include "channel-audio-engine.hpp"
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
    m_channels[ChannelType::GAME]   = { ChannelType::GAME,   "Audio Router - Game",   0.85f, 1.0f, false, {}, nullptr };
    m_channels[ChannelType::CHAT]   = { ChannelType::CHAT,   "Audio Router - Chat",   1.00f, 1.0f, false, {}, nullptr };
    m_channels[ChannelType::MEDIA]  = { ChannelType::MEDIA,  "Audio Router - Media",  0.50f, 0.8f, false, {}, nullptr };
    m_channels[ChannelType::SYSTEM] = { ChannelType::SYSTEM, "Audio Router - System", 0.70f, 0.7f, false, {}, nullptr };
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
}

float ChannelAudioEngine::getMonitorVolume(ChannelType channel) const {
    auto it = m_channels.find(channel);
    return (it != m_channels.end()) ? it->second.monitorVolume : 1.0f;
}

void ChannelAudioEngine::setMuted(ChannelType channel, bool muted) {
    m_channels[channel].isMuted = muted;
    if (m_channels[channel].obsSource) {
        obs_source_set_muted(m_channels[channel].obsSource, muted);
    }
}

bool ChannelAudioEngine::isMuted(ChannelType channel) const {
    auto it = m_channels.find(channel);
    return (it != m_channels.end()) ? it->second.isMuted : false;
}

void ChannelAudioEngine::assignProcessToChannel(const std::string& processName, ChannelType channel) {
    unassignProcess(processName);

    m_processChannelMap[processName] = channel;
    auto& list = m_channels[channel].assignedProcesses;
    if (std::find(list.begin(), list.end(), processName) == list.end()) {
        list.push_back(processName);
    }

    syncObsSourcesForChannel(channel);
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
