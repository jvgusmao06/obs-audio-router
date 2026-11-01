#include "ui/localization-helper.hpp"
#include <iostream>
#include <cassert>

int main() {
    auto& i18n = AudioRouter::I18nHelper::instance();

    std::cout << "[TEST] 1. Spanish (es-ES)..." << std::endl;
    i18n.loadLanguage("data/locale/es-ES.ini");
    assert(i18n.text("ChannelGame") == "Juego");
    assert(i18n.text("VolumeStream") == "Transmisión");
    assert(i18n.text("VolumeMonitor") == "Auriculares");

    std::cout << "[TEST] 2. French (fr-FR)..." << std::endl;
    i18n.loadLanguage("data/locale/fr-FR.ini");
    assert(i18n.text("ChannelGame") == "Jeu");
    assert(i18n.text("ChannelChat") == "Discussion");
    assert(i18n.text("VolumeMonitor") == "Casque");

    std::cout << "[TEST] 3. Russian (ru-RU)..." << std::endl;
    i18n.loadLanguage("data/locale/ru-RU.ini");
    assert(i18n.text("ChannelGame") == "Игра");
    assert(i18n.text("ChannelChat") == "Чат");
    assert(i18n.text("VolumeStream") == "Стрим");
    assert(i18n.text("VolumeMonitor") == "Наушники");

    std::cout << "[SUCCESS] All new languages (ES, FR, RU) loaded and verified successfully!" << std::endl;
    return 0;
}

