/**
 * @file test_i18n_and_rules.cpp
 * @brief Unit tests verifying multi-language locale loading, string fallbacks,
 *        custom channel routing rules, and JSON serialization.
 */

#include "ui/localization-helper.hpp"
#include "core/routing-rules.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[TEST] 1. Testando dicionario embutido na DLL sem arquivos externos (pt-BR)..." << std::endl;
    auto& i18n = AudioRouter::I18nHelper::instance();
    i18n.setLocale("pt-BR");

    assert(i18n.text("DockTitle") == "Audio Router");
    assert(i18n.text("ChannelGame") == "Jogo");
    assert(i18n.text("ChannelChat") == "Chat");
    assert(i18n.text("ChannelMedia") == "Mídia");
    assert(i18n.text("ChannelSystem") == "Sistema");
    assert(i18n.text("VolumeStream") == "Live");
    assert(i18n.text("VolumeMonitor") == "Fone");
    assert(i18n.text("MonitorStream") == "Ouvir Stream");
    assert(i18n.text("MonitoringStream") == "Ouvindo Stream");
    assert(i18n.text("MakeIgnored") == "Ignorar aplicativo");
    assert(i18n.text("IgnoredApps") == "Apps Ignorados");
    assert(i18n.text("RestoreApp") == "Restaurar");
    assert(i18n.text("DesktopAudioTip").rfind("Dica:", 0) == 0);

    std::cout << "[TEST] 2. Testando dicionario embutido em ingles (en-US)..." << std::endl;
    i18n.setLocale("en-US");
    assert(i18n.text("DockTitle") == "Audio Router");
    assert(i18n.text("ChannelGame") == "Game");
    assert(i18n.text("ChannelMedia") == "Media");
    assert(i18n.text("VolumeMonitor") == "Headphones");
    assert(i18n.text("MonitorStream") == "Monitor Stream");
    assert(i18n.text("MonitoringStream") == "Monitoring Stream");
    assert(i18n.text("MakeIgnored") == "Ignore App");
    assert(i18n.text("IgnoredApps") == "Ignored Apps");
    assert(i18n.text("RestoreApp") == "Restore");
    assert(i18n.text("DesktopAudioTip").rfind("Tip:", 0) == 0);

    std::cout << "[TEST] 3. Testando reatribuicao e persistencia de regra customizada..." << std::endl;
    AudioRouter::RoutingRules rules;
    rules.setCustomRule("spotify.exe", AudioRouter::ChannelType::CHAT);
    assert(rules.resolveChannel("spotify.exe") == AudioRouter::ChannelType::CHAT);

    std::string json = rules.serializeToJson();
    assert(json.find("\"spotify.exe\": \"Chat\"") != std::string::npos);

    std::cout << "[SUCCESS] Todos os testes de internacionalizacao e regras passaram!" << std::endl;
    return 0;
}
