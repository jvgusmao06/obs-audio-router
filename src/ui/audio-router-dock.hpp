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

private:
    void setupUi();
    void loadRulesFromDisk();
    void saveRulesToDisk();

    std::unique_ptr<RoutingRules> m_rules;
    std::unique_ptr<AudioSessionWatcher> m_sessionWatcher;

    ChatMixSlider *m_chatMixSlider = nullptr;
    QPushButton *m_listenStreamBtn = nullptr;
    QHBoxLayout *m_channelsLayout = nullptr;
    std::unordered_map<ChannelType, ChannelStripWidget*> m_channelStrips;
    std::unordered_map<std::string, bool> m_knownApps;
};

} // namespace AudioRouter
