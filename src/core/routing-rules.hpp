/**
 * @file routing-rules.hpp
 * @brief Categorization rules mapping process names to audio channels (Master, Game, Chat, Media, System)
 *        with JSON serialization for user overrides and ignored applications.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace AudioRouter {

enum class ChannelType {
    MASTER,
    GAME,
    CHAT,
    MEDIA,
    SYSTEM
};

inline const char* channelToString(ChannelType type) {
    switch (type) {
        case ChannelType::MASTER: return "Master";
        case ChannelType::GAME:   return "Jogo";
        case ChannelType::CHAT:   return "Chat";
        case ChannelType::MEDIA:  return "Mídia";
        case ChannelType::SYSTEM: return "Sistema";
        default:                  return "Desconhecido";
    }
}

inline ChannelType stringToChannel(const std::string& str) {
    if (str == "Master" || str == "MASTER") return ChannelType::MASTER;
    if (str == "Jogo" || str == "GAME")     return ChannelType::GAME;
    if (str == "Chat" || str == "CHAT")     return ChannelType::CHAT;
    if (str == "Mídia" || str == "MEDIA")   return ChannelType::MEDIA;
    return ChannelType::SYSTEM;
}

class RoutingRules {
public:
    RoutingRules();

    // Determina o canal para um determinado executável
    ChannelType resolveChannel(const std::string& processName) const;

    // Adiciona ou substitui regra para um processo específico
    void setCustomRule(const std::string& processName, ChannelType channel);

    // Remove regra customizada (volta para a regra default)
    void removeCustomRule(const std::string& processName);

    // Gerenciamento de aplicativos ignorados pelo router
    bool isIgnored(const std::string& processName) const;
    void setAppIgnored(const std::string& processName, bool ignored);
    std::vector<std::string> getIgnoredApps() const;

    // Serialização JSON simples para persistência em disco sem bibliotecas externas
    std::string serializeToJson() const;
    void deserializeFromJson(const std::string& json);

private:
    void initDefaultRules();
    void initDefaultIgnoredApps();
    static std::string toLower(std::string str);

    std::unordered_map<std::string, ChannelType> defaultRules;
    std::unordered_map<std::string, ChannelType> customRules;
    std::unordered_set<std::string> ignoredApps;
};

} // namespace AudioRouter
