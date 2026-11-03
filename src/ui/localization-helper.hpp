/**
 * @file localization-helper.hpp
 * @brief Lightweight internationalization (i18n) helper parsing INI locale files
 *        with built-in fallback dictionaries for true single-DLL standalone deployment.
 */

#pragma once

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>

namespace AudioRouter {

class I18nHelper {
public:
    static I18nHelper& instance() {
        static I18nHelper s_instance;
        return s_instance;
    }

    void setLocale(const std::string& localeCode) {
        m_locale = localeCode;
        initBuiltinStrings(localeCode);
    }

    void loadLanguage(const std::string& localeFilePath) {
        std::ifstream file(localeFilePath);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == ';' || line[0] == '#') continue;
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);

            // Remove quotes if present
            if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
                val = val.substr(1, val.size() - 2);
            }
            strings[key] = val;
        }
    }

    std::string text(const std::string& key, const std::string& defaultFallback = "") const {
        auto it = strings.find(key);
        if (it != strings.end()) {
            return it->second;
        }
        return defaultFallback.empty() ? key : defaultFallback;
    }

private:
    I18nHelper() {
        setLocale("en-US");
    }

    void initBuiltinStrings(const std::string& localeCode) {
        strings.clear();
        if (localeCode.rfind("pt", 0) == 0) {
            strings["ChannelGame"] = "Jogo";
            strings["ChannelChat"] = "Chat";
            strings["ChannelMedia"] = "Mídia";
            strings["ChannelSystem"] = "Sistema";
            strings["VolumeStream"] = "Live";
            strings["VolumeMonitor"] = "Fone";
            strings["ChatMix"] = "ChatMix";
            strings["ChatMixTooltip"] = "ChatMix: Balanço entre Jogo (esquerda) e Chat (direita). Clique duplo para centralizar.";
            strings["MonitorStream"] = "Ouvir Stream";
            strings["MonitoringStream"] = "Ouvindo Stream";
            strings["MonitorStreamTip"] = "Escute nos seus fones o áudio exatamente como está saindo para a stream";
            strings["Mute"] = "Mudo";
            strings["Muted"] = "Mudo";
            strings["Unmute"] = "Desmutar";
            strings["MoveTo"] = "Mover para";
            strings["Remove"] = "Remover";
            strings["DockTitle"] = "Audio Router";
            strings["MakeIgnored"] = "Ignorar aplicativo";
            strings["IgnoredApps"] = "Apps Ignorados";
            strings["IgnoredAppsTitle"] = "Aplicativos Ignorados";
            strings["IgnoredAppsSubtitle"] = "Aplicativos nesta lista não aparecem no Router e tocam direto no Windows sem alteração de volume.";
            strings["RestoreApp"] = "Restaurar";
            strings["NoIgnoredApps"] = "Nenhum aplicativo ignorado configurado.";
            strings["Close"] = "Fechar";
            strings["DesktopAudioTip"] = "Dica: No mixer do OBS, silencie o 'Áudio do Desktop' padrão para evitar eco, pois o Audio Router já envia os canais diretamente para a gravação/live.";
        } else if (localeCode.rfind("es", 0) == 0) {
            strings["ChannelGame"] = "Juego";
            strings["ChannelChat"] = "Chat";
            strings["ChannelMedia"] = "Medios";
            strings["ChannelSystem"] = "Sistema";
            strings["VolumeStream"] = "Transmisión";
            strings["VolumeMonitor"] = "Auriculares";
            strings["ChatMix"] = "ChatMix";
            strings["ChatMixTooltip"] = "ChatMix: Balance entre Juego (izquierda) e Chat (derecha). Doble clic para centrar.";
            strings["MonitorStream"] = "Escuchar Stream";
            strings["MonitoringStream"] = "Escuchando Stream";
            strings["MonitorStreamTip"] = "Escuche en sus auriculares el audio exactamente como sale al stream";
            strings["Mute"] = "Silenciar";
            strings["Muted"] = "Silenciado";
            strings["Unmute"] = "Reactivar";
            strings["MoveTo"] = "Mover a";
            strings["Remove"] = "Eliminar";
            strings["DockTitle"] = "Audio Router";
            strings["MakeIgnored"] = "Ignorar aplicación";
            strings["IgnoredApps"] = "Apps Ignoradas";
            strings["IgnoredAppsTitle"] = "Aplicaciones Ignoradas";
            strings["IgnoredAppsSubtitle"] = "Las aplicaciones en esta lista no aparecen en el Router y se reproducen en Windows sin alteración de volumen.";
            strings["RestoreApp"] = "Restaurar";
            strings["NoIgnoredApps"] = "No hay aplicaciones ignoradas configuradas.";
            strings["Close"] = "Cerrar";
            strings["DesktopAudioTip"] = "Consejo: En el mezclador de OBS, silencia el 'Audio del escritorio' por defecto para evitar eco, ya que Audio Router envía los canales directamente a la transmisión/grabación.";
        } else if (localeCode.rfind("fr", 0) == 0) {
            strings["ChannelGame"] = "Jeu";
            strings["ChannelChat"] = "Discussion";
            strings["ChannelMedia"] = "Médias";
            strings["ChannelSystem"] = "Système";
            strings["VolumeStream"] = "Direct";
            strings["VolumeMonitor"] = "Casque";
            strings["ChatMix"] = "ChatMix";
            strings["ChatMixTooltip"] = "ChatMix: Équilibre entre Jeu (gauche) et Chat (droite). Double-clic pour centrer.";
            strings["MonitorStream"] = "Écouter le Stream";
            strings["MonitoringStream"] = "Écoute du Stream";
            strings["MonitorStreamTip"] = "Écoutez dans votre casque le son tel qu'il sort sur le stream";
            strings["Mute"] = "Muet";
            strings["Muted"] = "Muet";
            strings["Unmute"] = "Activer";
            strings["MoveTo"] = "Déplacer vers";
            strings["Remove"] = "Supprimer";
            strings["DockTitle"] = "Audio Router";
            strings["MakeIgnored"] = "Ignorer l'application";
            strings["IgnoredApps"] = "Apps Ignorées";
            strings["IgnoredAppsTitle"] = "Applications Ignorées";
            strings["IgnoredAppsSubtitle"] = "Les applications de cette liste n'apparaissent pas dans le Router et jouent directement dans Windows sans modification de volume.";
            strings["RestoreApp"] = "Restaurer";
            strings["NoIgnoredApps"] = "Aucune application ignorée configurée.";
            strings["Close"] = "Fermer";
            strings["DesktopAudioTip"] = "Astuce : Dans le mélangeur OBS, coupez le son de 'l'Audio du bureau' par défaut pour éviter l'écho, car Audio Router envoie les canaux directement au stream/enregistrement.";
        } else if (localeCode.rfind("ru", 0) == 0) {
            strings["ChannelGame"] = "Игра";
            strings["ChannelChat"] = "Чат";
            strings["ChannelMedia"] = "Медиа";
            strings["ChannelSystem"] = "Система";
            strings["VolumeStream"] = "Стрим";
            strings["VolumeMonitor"] = "Наушники";
            strings["ChatMix"] = "ChatMix";
            strings["ChatMixTooltip"] = "ChatMix: Баланс между Игра (слева) и Чат (справа). Двойной клик для центрирования.";
            strings["MonitorStream"] = "Слушать стрим";
            strings["MonitoringStream"] = "Слушаю стрим";
            strings["MonitorStreamTip"] = "Прослушивайте в наушниках звук именно так, как он идет на стрим";
            strings["Mute"] = "Заглушить";
            strings["Muted"] = "Заглушено";
            strings["Unmute"] = "Включить звук";
            strings["MoveTo"] = "Переместить в";
            strings["Remove"] = "Удалить";
            strings["DockTitle"] = "Audio Router";
            strings["MakeIgnored"] = "Игнорировать приложение";
            strings["IgnoredApps"] = "Игнорируемые";
            strings["IgnoredAppsTitle"] = "Игнорируемые приложения";
            strings["IgnoredAppsSubtitle"] = "Приложения из этого списка не отображаются в Router и играют напрямую в Windows без изменения громкости.";
            strings["RestoreApp"] = "Восстановить";
            strings["NoIgnoredApps"] = "Нет игнорируемых приложений.";
            strings["Close"] = "Закрыть";
            strings["DesktopAudioTip"] = "Совет: В микшере OBS отключите стандартный «Аудио с рабочего стола», чтобы избежать эха, так как Audio Router передает каналы напрямую в трансляцию/запись.";
        } else {
            // Default English
            strings["ChannelGame"] = "Game";
            strings["ChannelChat"] = "Chat";
            strings["ChannelMedia"] = "Media";
            strings["ChannelSystem"] = "System";
            strings["VolumeStream"] = "Stream";
            strings["VolumeMonitor"] = "Headphones";
            strings["ChatMix"] = "ChatMix";
            strings["ChatMixTooltip"] = "ChatMix: Balance between Game (left) and Chat (right). Double click to center.";
            strings["MonitorStream"] = "Monitor Stream";
            strings["MonitoringStream"] = "Monitoring Stream";
            strings["MonitorStreamTip"] = "Listen in your headphones to the audio exactly as it outputs to the stream";
            strings["Mute"] = "Mute";
            strings["Muted"] = "Muted";
            strings["Unmute"] = "Unmute";
            strings["MoveTo"] = "Move to";
            strings["Remove"] = "Remove";
            strings["DockTitle"] = "Audio Router";
            strings["MakeIgnored"] = "Ignore App";
            strings["IgnoredApps"] = "Ignored Apps";
            strings["IgnoredAppsTitle"] = "Ignored Applications";
            strings["IgnoredAppsSubtitle"] = "Applications in this list do not appear in Router and output directly to Windows without volume alterations.";
            strings["RestoreApp"] = "Restore";
            strings["NoIgnoredApps"] = "No ignored applications configured.";
            strings["Close"] = "Close";
            strings["DesktopAudioTip"] = "Tip: In OBS mixer, mute default 'Desktop Audio' to avoid echo, as Audio Router sends channels directly to your broadcast/recording.";
        }
    }

    std::string m_locale = "en-US";
    std::unordered_map<std::string, std::string> strings;
};

inline std::string loc(const std::string& key, const std::string& defaultVal = "") {
    return I18nHelper::instance().text(key, defaultVal);
}

} // namespace AudioRouter
