#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace AudioRouter {

enum class ChannelType {
    GAME,
    CHAT,
    MEDIA,
    SYSTEM
};

inline const char* channelToString(ChannelType type) {
    switch (type) {
        case ChannelType::GAME:   return "Jogo";
        case ChannelType::CHAT:   return "Chat";
        case ChannelType::MEDIA:  return "Mídia";
        case ChannelType::SYSTEM: return "Sistema";
        default:                  return "Desconhecido";
    }
}

inline ChannelType stringToChannel(const std::string& str) {
    if (str == "Jogo" || str == "GAME")   return ChannelType::GAME;
    if (str == "Chat" || str == "CHAT")   return ChannelType::CHAT;
    if (str == "Mídia" || str == "MEDIA") return ChannelType::MEDIA;
    return ChannelType::SYSTEM;
}

class RoutingRules {
public:
    RoutingRules();

    // Retorna o canal atribuído para um executável (ex: "discord.exe")
    ChannelType resolveChannel(const std::string& executableName) const;

    // Associa manualmente um processo a um canal (persistente)
    void setCustomRule(const std::string& executableName, ChannelType channel);

    // Remove regra personalizada voltando ao padrão
    void removeCustomRule(const std::string& executableName);

    // Serialização JSON simples para salvar configurações
    std::string serializeToJson() const;
    void deserializeFromJson(const std::string& jsonString);

private:
    std::unordered_map<std::string, ChannelType> defaultRules;
    std::unordered_map<std::string, ChannelType> customRules;

    void initDefaultRules();
    static std::string toLower(std::string str);
};

} // namespace AudioRouter

