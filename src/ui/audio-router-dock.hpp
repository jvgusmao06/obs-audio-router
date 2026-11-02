#pragma once

#include <QWidget>
#include <memory>
#include <unordered_map>
#include "core/routing-rules.hpp"
#include "core/audio-session-watcher.hpp"
#include "channel-drop-zone.hpp"
#include "chatmix-slider.hpp"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;

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
    QWidget* createChannelColumn(const QString& titleKey, ChannelType type);

    std::unique_ptr<RoutingRules> m_rules;
    std::unique_ptr<AudioSessionWatcher> m_sessionWatcher;

    QHBoxLayout *m_channelsLayout = nullptr;
    QHBoxLayout *m_activeAppsLayout = nullptr;
    std::unordered_map<ChannelType, ChannelDropZone*> m_dropZones;
    std::unordered_map<std::string, bool> m_knownApps;
};

} // namespace AudioRouter
