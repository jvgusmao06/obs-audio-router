/**
 * @file channel-audio-engine.hpp
 * @brief Core mixing engine managing OBS audio channels, ChatMix attenuation,
 *        Stream vs. Monitor separation, Master bus scaling, and Stream Monitoring mode.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "routing-rules.hpp"

// Tipos opacos da libobs caso compilado fora do header completo
struct obs_source;
typedef struct obs_source obs_source_t;
struct obs_data;
typedef struct obs_data obs_data_t;

namespace AudioRouter {

struct ChannelAudioState {
    ChannelType type;
    std::string channelName;
    float streamVolume = 0.85f;    // 0.0f a 1.0f (ou ganho linear)
    float monitorVolume = 1.0f;   // 0.0f a 1.0f
    bool isStreamMuted = false;
    bool isMonitorMuted = false;
    
    // Lista de fontes de áudio de processo ativas associadas a este canal
    std::vector<std::string> assignedProcesses;
    
    // Ponteiro da fonte OBS gerenciada (Virtual Source / Bus)
    obs_source_t *obsSource = nullptr;
};

class ChannelAudioEngine {
public:
    static ChannelAudioEngine& instance();

    void initializeChannels();
    void shutdownChannels();

    // Volume da transmissão (Live/Stream)
    void setStreamVolume(ChannelType channel, float volumeNormalized);
    float getStreamVolume(ChannelType channel) const;

    // Volume do fone de ouvido (Monitor/Headphones)
    void setMonitorVolume(ChannelType channel, float volumeNormalized);
    float getMonitorVolume(ChannelType channel) const;
    float getEffectiveMonitorVolume(ChannelType channel) const;

    // ChatMix (-1.0f = 100% Game, 0.0f = Centro, +1.0f = 100% Chat)
    void setChatMix(float factor);
    float getChatMix() const;
    static std::pair<float, float> calculateChatMixGains(float chatMixFactor, float gameVol, float chatVol);

    // Vincula o SessionWatcher para refletir volume nas sessões WASAPI do Windows
    void setSessionWatcher(class AudioSessionWatcher *watcher);
    class AudioSessionWatcher* getSessionWatcher() const;

    // Controle de Mudo Independente
    void setStreamMuted(ChannelType channel, bool muted);
    bool isStreamMuted(ChannelType channel) const;

    void setMonitorMuted(ChannelType channel, bool muted);
    bool isMonitorMuted(ChannelType channel) const;

    // Compatibilidade anterior
    void setMuted(ChannelType channel, bool muted) { setStreamMuted(channel, muted); setMonitorMuted(channel, muted); }
    bool isMuted(ChannelType channel) const { return isMonitorMuted(channel); }

    // Modo Ouvir Stream (Permite ao streamer escutar o mix final da live nos fones)
    void setMonitoringStream(bool monitoring);
    bool isMonitoringStream() const;

    // Retorna o pico instantâneo de áudio de um canal (0.0f a 1.0f) para os VU meters
    float getChannelPeak(ChannelType channel) const;

    // Roteamento: Associa ou remove um processo de um canal
    void assignProcessToChannel(const std::string& processName, ChannelType channel);
    void unassignProcess(const std::string& processName);
    obs_source_t* getProcessSource(const std::string& processName) const;

    // Retorna o estado de um canal
    const ChannelAudioState* getChannelState(ChannelType channel) const;

private:
    ChannelAudioEngine();
    ~ChannelAudioEngine();

    void syncObsSourcesForChannel(ChannelType channel);
    void applyVolumeToProcesses(ChannelType channel);
    void applyMuteToProcesses(ChannelType channel);

    float m_chatMix = 0.0f;
    bool m_isMonitoringStream = false;
    class AudioSessionWatcher *m_sessionWatcher = nullptr;
    std::unordered_map<ChannelType, ChannelAudioState> m_channels;
    std::unordered_map<std::string, ChannelType> m_processChannelMap;
    std::unordered_map<std::string, obs_source_t*> m_processSources;
};

} // namespace AudioRouter
