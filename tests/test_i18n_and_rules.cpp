#include "ui/localization-helper.hpp"
#include "core/routing-rules.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "[TEST] 1. Testando carregamento de arquivo de idioma (pt-BR)..." << std::endl;
    auto& i18n = AudioRouter::I18nHelper::instance();
    i18n.loadLanguage("data/locale/pt-BR.ini");

    assert(i18n.text("DockTitle") == "Audio Router");
    assert(i18n.text("ChannelGame") == "Jogo");
    assert(i18n.text("ChannelChat") == "Chat");
    assert(i18n.text("ChannelMedia") == "Mídia");
    assert(i18n.text("ChannelSystem") == "Sistema");
    assert(i18n.text("VolumeStream") == "Live");
    assert(i18n.text("VolumeMonitor") == "Fone");
    assert(i18n.text("MonitorStream") == "Ouvir Stream");
    assert(i18n.text("MonitoringStream") == "Ouvindo Stream");

    std::cout << "[TEST] 2. Testando fallback para idioma inglês (en-US)..." << std::endl;
    i18n.loadLanguage("data/locale/en-US.ini");
    assert(i18n.text("DockTitle") == "Audio Router");
    assert(i18n.text("ChannelGame") == "Game");
    assert(i18n.text("ChannelMedia") == "Media");
    assert(i18n.text("VolumeMonitor") == "Headphones");
    assert(i18n.text("MonitorStream") == "Monitor Stream");
    assert(i18n.text("MonitoringStream") == "Monitoring Stream");

    std::cout << "[TEST] 3. Testando reatribuição e persistência de regra customizada..." << std::endl;
    AudioRouter::RoutingRules rules;
    rules.setCustomRule("spotify.exe", AudioRouter::ChannelType::CHAT);
    assert(rules.resolveChannel("spotify.exe") == AudioRouter::ChannelType::CHAT);

    std::string json = rules.serializeToJson();
    assert(json.find("\"spotify.exe\": \"Chat\"") != std::string::npos);

    std::cout << "[SUCCESS] Todos os testes de internacionalização e regras passaram!" << std::endl;
    return 0;
}
