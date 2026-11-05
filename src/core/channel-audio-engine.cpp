/**
 * @file channel-audio-engine.cpp
 * @brief Implementation of ChannelAudioEngine managing audio routing to OBS sources
 *        and applying hardware volume/mute states to Windows audio sessions.
 */

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
        extern void obs_source_inc_active(obs_source_t *source);
        extern void obs_source_dec_active(obs_source_t *source);
        extern void obs_source_remove(obs_source_t *source);
        extern void obs_source_set_volume(obs_source_t *source, float volume);
        extern float obs_source_get_volume(const obs_source_t *source);
        extern void obs_source_set_muted(obs_source_t *source, bool muted);
        extern bool obs_source_muted(const obs_source_t *source);
        extern void obs_source_set_monitoring_type(obs_source_t *source, enum obs_monitoring_type type);
        extern obs_data_t* obs_data_create(void);
        extern void obs_data_set_string(obs_data_t *data, const char *name, const char *val);
        extern void obs_data_set_int(obs_data_t *data, const char *name, long long val);
        extern void obs_data_release(obs_data_t *data);
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
    m_channels[ChannelType::MASTER] = { ChannelType::MASTER, "Audio Router - Master", 1.00f, 1.0f, false, false, {}, nullptr };
    m_channels[ChannelType::GAME]   = { ChannelType::GAME,   "Audio Router - Game",   0.85f, 1.0f, false, false, {}, nullptr };
    m_channels[ChannelType::CHAT]   = { ChannelType::CHAT,   "Audio Router - Chat",   1.00f, 1.0f, false, false, {}, nullptr };
    m_channels[ChannelType::MEDIA]  = { ChannelType::MEDIA,  "Audio Router - Media",  0.50f, 0.8f, false, false, {}, nullptr };
    m_channels[ChannelType::SYSTEM] = { ChannelType::SYSTEM, "Audio Router - System", 0.70f, 0.7f, false, false, {}, nullptr };
}

void ChannelAudioEngine::shutdownChannels() {
    m_isShuttingDown = true;
    for (auto& [name, source] : m_processSources) {
        if (source) {
            obs_source_dec_active(source);
            obs_source_remove(source);
            obs_source_release(source);
        }
    }
    m_processSources.clear();
    m_processChannelMap.clear();

    for (auto& [type, state] : m_channels) {
        state.assignedProcesses.clear();
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

    applyVolumeToProcesses(channel);
    applyMuteToProcesses(channel);
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

void ChannelAudioEngine::setMonitoringStream(bool monitoring) {
    if (m_isMonitoringStream == monitoring) return;
    m_isMonitoringStream = monitoring;
    for (auto type : {ChannelType::GAME, ChannelType::CHAT, ChannelType::MEDIA, ChannelType::SYSTEM}) {
        applyVolumeToProcesses(type);
        applyMuteToProcesses(type);
    }
}

bool ChannelAudioEngine::isMonitoringStream() const {
    return m_isMonitoringStream;
}

float ChannelAudioEngine::getChannelPeak(ChannelType channel) const {
    if (!m_sessionWatcher) return 0.0f;

    bool masterMuted = m_isMonitoringStream ? isStreamMuted(ChannelType::MASTER) : isMonitorMuted(ChannelType::MASTER);
    float masterVol = m_isMonitoringStream ? getStreamVolume(ChannelType::MASTER) : getEffectiveMonitorVolume(ChannelType::MASTER);

    if (masterMuted || masterVol <= 0.001f) return 0.0f;

    if (channel == ChannelType::MASTER) {
        float maxPeak = 0.0f;
        for (auto subType : {ChannelType::GAME, ChannelType::CHAT, ChannelType::MEDIA, ChannelType::SYSTEM}) {
            float subPeak = getChannelPeak(subType);
            if (subPeak > maxPeak) maxPeak = subPeak;
        }
        return maxPeak;
    }

    auto it = m_channels.find(channel);
    if (it == m_channels.end()) return 0.0f;

    bool channelMuted = m_isMonitoringStream ? it->second.isStreamMuted : it->second.isMonitorMuted;
    if (channelMuted) return 0.0f;

    float channelVol = m_isMonitoringStream ? getStreamVolume(channel) : getEffectiveMonitorVolume(channel);
    if (channelVol <= 0.001f) return 0.0f;

    float maxProcPeak = 0.0f;
    for (const auto& proc : it->second.assignedProcesses) {
        float procPeak = m_sessionWatcher->getProcessPeakVolume(proc);
        if (procPeak > maxProcPeak) maxProcPeak = procPeak;
    }

    return std::clamp(maxProcPeak * channelVol * masterVol, 0.0f, 1.0f);
}

void ChannelAudioEngine::applyVolumeToProcesses(ChannelType channel) {
    if (m_isShuttingDown) return;
    if (channel == ChannelType::MASTER) {
        for (auto type : {ChannelType::GAME, ChannelType::CHAT, ChannelType::MEDIA, ChannelType::SYSTEM}) {
            applyVolumeToProcesses(type);
        }
        return;
    }

    auto it = m_channels.find(channel);
    if (it == m_channels.end()) return;

    float masterStream = getStreamVolume(ChannelType::MASTER);
    float chanStream = it->second.streamVolume;
    float effectiveStreamVolume = std::clamp(chanStream * masterStream, 0.0f, 1.0f);
    bool effectiveStreamMuted = it->second.isStreamMuted || isStreamMuted(ChannelType::MASTER);

    float masterMon = getEffectiveMonitorVolume(ChannelType::MASTER);
    float chanMon = getEffectiveMonitorVolume(channel);
    float effectiveMonitorVolume = std::clamp(chanMon * masterMon, 0.0f, 1.0f);
    bool effectiveMonitorMuted = it->second.isMonitorMuted || isMonitorMuted(ChannelType::MASTER);

    float streamGain = std::pow(effectiveStreamVolume, 3.0f);

    for (const auto& proc : it->second.assignedProcesses) {
        obs_source_t* source = getProcessSource(proc);
        if (source) {
            obs_source_set_volume(source, streamGain);
            obs_source_set_muted(source, effectiveStreamMuted);
            obs_source_set_monitoring_type(source, OBS_MONITORING_TYPE_NONE);


        }

        if (m_sessionWatcher) {
            float effectiveVol = m_isMonitoringStream ? effectiveStreamVolume : effectiveMonitorVolume;
            m_sessionWatcher->setProcessVolume(proc, std::clamp(effectiveVol, 0.0f, 1.0f));
        }
    }
}

void ChannelAudioEngine::applyMuteToProcesses(ChannelType channel) {
    if (m_isShuttingDown) return;
    if (channel == ChannelType::MASTER) {
        for (auto type : {ChannelType::GAME, ChannelType::CHAT, ChannelType::MEDIA, ChannelType::SYSTEM}) {
            applyMuteToProcesses(type);
        }
        return;
    }

    auto it = m_channels.find(channel);
    if (it == m_channels.end()) return;

    bool masterMuted = m_isMonitoringStream ? isStreamMuted(ChannelType::MASTER) : isMonitorMuted(ChannelType::MASTER);
    bool channelMuted = m_isMonitoringStream ? it->second.isStreamMuted : it->second.isMonitorMuted;
    bool effectiveMuted = masterMuted || channelMuted;

    if (m_sessionWatcher) {
        for (const auto& proc : it->second.assignedProcesses) {
            m_sessionWatcher->setProcessMute(proc, effectiveMuted);
        }
    }
}

void ChannelAudioEngine::setStreamMuted(ChannelType channel, bool muted) {
    m_channels[channel].isStreamMuted = muted;
    if (m_channels[channel].obsSource) {
        obs_source_set_muted(m_channels[channel].obsSource, muted);
    }
    applyVolumeToProcesses(channel);
    applyMuteToProcesses(channel);
}

bool ChannelAudioEngine::isStreamMuted(ChannelType channel) const {
    auto it = m_channels.find(channel);
    return (it != m_channels.end()) ? it->second.isStreamMuted : false;
}

void ChannelAudioEngine::setMonitorMuted(ChannelType channel, bool muted) {
    m_channels[channel].isMonitorMuted = muted;
    applyVolumeToProcesses(channel);
    applyMuteToProcesses(channel);
}

bool ChannelAudioEngine::isMonitorMuted(ChannelType channel) const {
    auto it = m_channels.find(channel);
    return (it != m_channels.end()) ? it->second.isMonitorMuted : false;
}

obs_source_t* ChannelAudioEngine::getProcessSource(const std::string& processName) const {
    auto it = m_processSources.find(processName);
    return (it != m_processSources.end()) ? it->second : nullptr;
}

void ChannelAudioEngine::assignProcessToChannel(const std::string& processName, ChannelType channel) {
    if (m_isShuttingDown || processName.empty()) {
        return;
    }

    auto mapIt = m_processChannelMap.find(processName);
    if (mapIt != m_processChannelMap.end() && mapIt->second == channel) {
        if (processName == "System Sounds" || m_processSources.find(processName) != m_processSources.end()) {
            applyVolumeToProcesses(channel);
            applyMuteToProcesses(channel);
            return;
        }
    }

    unassignProcess(processName);

    m_processChannelMap[processName] = channel;
    auto& list = m_channels[channel].assignedProcesses;
    if (std::find(list.begin(), list.end(), processName) == list.end()) {
        list.push_back(processName);
    }

    if (processName != "System Sounds") {
        obs_data_t *settings = obs_data_create();
        // O OBS ms_build_window_strings espera o formato [title]:[class]:[executable].
        // Passando "::" + processName garante que executable seja preenchido e class não seja nula.
        std::string windowTarget = "::" + processName;
        obs_data_set_string(settings, "window", windowTarget.c_str());
        obs_data_set_int(settings, "priority", 2); // 2 = WINDOW_PRIORITY_EXE in OBS
        obs_source_t *source = obs_source_create("wasapi_process_output_capture", processName.c_str(), settings, nullptr);
        obs_data_release(settings);

        if (source) {
            obs_source_set_monitoring_type(source, OBS_MONITORING_TYPE_NONE);
            obs_source_inc_active(source);
            m_processSources[processName] = source;
        }
    }

    applyVolumeToProcesses(channel);
    applyMuteToProcesses(channel);
}

void ChannelAudioEngine::unassignProcess(const std::string& processName) {
    auto srcIt = m_processSources.find(processName);
    if (srcIt != m_processSources.end()) {
        if (srcIt->second) {
            obs_source_dec_active(srcIt->second);
            obs_source_remove(srcIt->second);
            obs_source_release(srcIt->second);
        }
        m_processSources.erase(srcIt);
    }

    auto it = m_processChannelMap.find(processName);
    if (it != m_processChannelMap.end()) {
        ChannelType oldChannel = it->second;
        auto& list = m_channels[oldChannel].assignedProcesses;
        list.erase(std::remove(list.begin(), list.end(), processName), list.end());
        m_processChannelMap.erase(it);
    }
}

const ChannelAudioState* ChannelAudioEngine::getChannelState(ChannelType channel) const {
    auto it = m_channels.find(channel);
    return (it != m_channels.end()) ? &it->second : nullptr;
}

} // namespace AudioRouter
