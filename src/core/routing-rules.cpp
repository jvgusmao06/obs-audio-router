/**
 * @file routing-rules.cpp
 * @brief Implementation of application channel routing rules, default application lists,
 *        ignored applications management, and JSON configuration serialization/deserialization.
 */

#include "routing-rules.hpp"
#include <algorithm>
#include <sstream>

namespace AudioRouter {

RoutingRules::RoutingRules() {
    initDefaultRules();
    initDefaultIgnoredApps();
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

    // 🎮 JOGO: Exemplos de jogos (launchers como Steam/Epic/Riot vão para SYSTEM por padrão)
    defaultRules["valorant.exe"] = ChannelType::GAME;
    defaultRules["cs2.exe"] = ChannelType::GAME;
}

void RoutingRules::initDefaultIgnoredApps() {
    ignoredApps.insert("obs64.exe");
    ignoredApps.insert("obs32.exe");
    ignoredApps.insert("obs.exe");
}

bool RoutingRules::isIgnored(const std::string& processName) const {
    if (processName.empty()) return false;
    std::string key = toLower(processName);
    return ignoredApps.find(key) != ignoredApps.end();
}

void RoutingRules::setAppIgnored(const std::string& processName, bool ignored) {
    if (processName.empty()) return;
    std::string key = toLower(processName);
    if (ignored) {
        ignoredApps.insert(key);
    } else {
        ignoredApps.erase(key);
    }
}

std::vector<std::string> RoutingRules::getIgnoredApps() const {
    std::vector<std::string> list(ignoredApps.begin(), ignoredApps.end());
    std::sort(list.begin(), list.end());
    return list;
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
    ss << "\n  },\n  \"ignored_apps\": [\n";
    first = true;
    auto sortedIgnored = getIgnoredApps();
    for (const auto& app : sortedIgnored) {
        if (!first) ss << ",\n";
        ss << "    \"" << app << "\"";
        first = false;
    }
    ss << "\n  ]\n}";
    return ss.str();
}

void RoutingRules::deserializeFromJson(const std::string& jsonString) {
    customRules.clear();
    ignoredApps.clear();
    initDefaultIgnoredApps();

    enum class Section { None, CustomRules, IgnoredApps };
    Section currentSection = Section::None;

    std::istringstream stream(jsonString);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.find("\"custom_rules\"") != std::string::npos) {
            currentSection = Section::CustomRules;
            continue;
        }
        if (line.find("\"ignored_apps\"") != std::string::npos || line.find("\"immune_apps\"") != std::string::npos) {
            currentSection = Section::IgnoredApps;
            continue;
        }

        if (currentSection == Section::CustomRules) {
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
        } else if (currentSection == Section::IgnoredApps) {
            auto firstQuote = line.find('"');
            if (firstQuote == std::string::npos) continue;
            auto secondQuote = line.find('"', firstQuote + 1);
            if (secondQuote == std::string::npos) continue;

            std::string app = line.substr(firstQuote + 1, secondQuote - firstQuote - 1);
            if (!app.empty()) {
                ignoredApps.insert(toLower(app));
            }
        } else {
            auto colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                auto firstQuote = line.find('"');
                auto secondQuote = line.find('"', firstQuote + 1);
                if (firstQuote != std::string::npos && secondQuote != std::string::npos && secondQuote < colonPos) {
                    auto thirdQuote = line.find('"', colonPos + 1);
                    auto fourthQuote = line.find('"', thirdQuote + 1);
                    if (thirdQuote != std::string::npos && fourthQuote != std::string::npos) {
                        std::string app = line.substr(firstQuote + 1, secondQuote - firstQuote - 1);
                        std::string channel = line.substr(thirdQuote + 1, fourthQuote - thirdQuote - 1);
                        customRules[toLower(app)] = stringToChannel(channel);
                    }
                }
            }
        }
    }
}

} // namespace AudioRouter
