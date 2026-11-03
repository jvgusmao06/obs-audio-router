/**
 * @file audio-router-dock.hpp
 * @brief Header for AudioRouterDock, the main OBS Studio docking panel containing
 *        the ChatMix slider, Stream Monitoring toggle, ignored apps manager, and channel mixer strips.
 */

#pragma once

#include <QWidget>
#include <memory>
#include <unordered_map>
#include "core/routing-rules.hpp"
#include "core/audio-session-watcher.hpp"
#include "channel-strip-widget.hpp"
#include "chatmix-slider.hpp"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QTimer;

namespace AudioRouter {

class AudioRouterDock : public QWidget {
    Q_OBJECT

public:
    explicit AudioRouterDock(QWidget *parent = nullptr);
    ~AudioRouterDock() override;

    // Atualiza a lista de aplicativos ativos detectados no Windows
    void refreshActiveApplications();

public slots:
    void onNewSessionDetected(const QString& appName, uint32_t pid);
    void onAppAssignedToChannel(const QString& appName, ChannelType channel);
    void onAppMadeIgnored(const QString& appName);
    void onAppIgnoredRemoved(const QString& appName);
    void showIgnoredAppsDialog();
    void updateVuMeters();

private:
    void setupUi();
    void loadRulesFromDisk();
    void saveRulesToDisk();
    void updateIgnoredBtnText();

    std::unique_ptr<RoutingRules> m_rules;
    std::unique_ptr<AudioSessionWatcher> m_sessionWatcher;

    ChatMixSlider *m_chatMixSlider = nullptr;
    QPushButton *m_listenStreamBtn = nullptr;
    QPushButton *m_ignoredAppsBtn = nullptr;
    QHBoxLayout *m_channelsLayout = nullptr;
    QTimer *m_meterTimer = nullptr;
    QTimer *m_sessionSyncTimer = nullptr;
    std::unordered_map<ChannelType, ChannelStripWidget*> m_channelStrips;
    std::unordered_map<std::string, bool> m_knownApps;
};

} // namespace AudioRouter
