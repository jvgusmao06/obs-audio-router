#include "routing-rules.hpp"
#include <algorithm>
#include <sstream>

namespace AudioRouter {

RoutingRules::RoutingRules() {
    initDefaultRules();
}

std::string RoutingRules::toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return str;
}

void RoutingRules::initDefaultRules() {
    // 💬 CHAT: Comunicação por voz e mensagens
    defaultRules["discord.exe"] = ChannelType::CHAT;
    defaultRules["discordptb.exe"] = ChannelType::CHAT;
    defaultRules["discordcanary.exe"] = ChannelType::CHAT;
    defaultRules["ts3client_win64.exe"] = ChannelType::CHAT;
    defaultRules["teamspeak.exe"] = ChannelType::CHAT;
    defaultRules["slack.exe"] = ChannelType::CHAT;
    defaultRules["telegram.exe"] = ChannelType::CHAT;
    defaultRules["whatsapp.exe"] = ChannelType::CHAT;
    defaultRules["teams.exe"] = ChannelType::CHAT;
    defaultRules["skype.exe"] = ChannelType::CHAT;

    // 🎵 MÍDIA: Players de áudio, streaming de música e vídeo
    defaultRules["spotify.exe"] = ChannelType::MEDIA;
    defaultRules["vlc.exe"] = ChannelType::MEDIA;
    defaultRules["foobar2000.exe"] = ChannelType::MEDIA;
    defaultRules["music.ui.exe"] = ChannelType::MEDIA; // Windows Media Player
    defaultRules["tidal.exe"] = ChannelType::MEDIA;
    defaultRules["itunes.exe"] = ChannelType::MEDIA;
    defaultRules["aimp.exe"] = ChannelType::MEDIA;

    // 🎮 JOGO: Launchers conhecidos ou jogos de exemplo
    defaultRules["steam.exe"] = ChannelType::GAME;
    defaultRules["steamwebhelper.exe"] = ChannelType::GAME;
    defaultRules["epicgameslauncher.exe"] = ChannelType::GAME;
    defaultRules["riotclientservices.exe"] = ChannelType::GAME;
    defaultRules["leagueclient.exe"] = ChannelType::GAME;
    defaultRules["valorant.exe"] = ChannelType::GAME;
    defaultRules["cs2.exe"] = ChannelType::GAME;
}

ChannelType RoutingRules::resolveChannel(const std::string& executableName) const {
    std::string key = toLower(executableName);

    // 1. Prioridade para regras customizadas definidas pelo usuário
    auto customIt = customRules.find(key);
    if (customIt != customRules.end()) {
        return customIt->second;
    }

    // 2. Regras padrão pré-configuradas (out-of-the-box)
    auto defaultIt = defaultRules.find(key);
    if (defaultIt != defaultRules.end()) {
        return defaultIt->second;
    }

    // 3. Fallback: Canal de Sistema
    return ChannelType::SYSTEM;
}

void RoutingRules::setCustomRule(const std::string& executableName, ChannelType channel) {
    customRules[toLower(executableName)] = channel;
}

void RoutingRules::removeCustomRule(const std::string& executableName) {
    customRules.erase(toLower(executableName));
}

std::string RoutingRules::serializeToJson() const {
    std::ostringstream ss;
    ss << "{\n  \"custom_rules\": {\n";
    bool first = true;
    for (const auto& [app, channel] : customRules) {
        if (!first) ss << ",\n";
        ss << "    \"" << app << "\": \"" << channelToString(channel) << "\"";
        first = false;
    }
    ss << "\n  }\n}";
    return ss.str();
}

void RoutingRules::deserializeFromJson(const std::string& jsonString) {
    customRules.clear();
    // Parse básico chave-valor para evitar dependências pesadas
    std::istringstream stream(jsonString);
    std::string line;
    while (std::getline(stream, line)) {
        auto colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;

        auto firstQuote = line.find('"');
        auto secondQuote = line.find('"', firstQuote + 1);
        if (firstQuote == std::string::npos || secondQuote == std::string::npos || secondQuote > colonPos) continue;

        auto thirdQuote = line.find('"', colonPos + 1);
        auto fourthQuote = line.find('"', thirdQuote + 1);
        if (thirdQuote == std::string::npos || fourthQuote == std::string::npos) continue;

        std::string app = line.substr(firstQuote + 1, secondQuote - firstQuote - 1);
        std::string channel = line.substr(thirdQuote + 1, fourthQuote - thirdQuote - 1);
        customRules[toLower(app)] = stringToChannel(channel);
    }
}

} // namespace AudioRouter

