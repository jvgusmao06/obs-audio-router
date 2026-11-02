#pragma once

#include <QFrame>
#include <QString>
#include <unordered_map>
#include "core/routing-rules.hpp"

class QVBoxLayout;
class QHBoxLayout;
class QSlider;
class QLabel;
class QPushButton;
class QScrollArea;

namespace AudioRouter {

class AppBadgeWidget;

class ChannelStripWidget : public QFrame {
    Q_OBJECT

public:
    explicit ChannelStripWidget(ChannelType type, QWidget *parent = nullptr);
    ~ChannelStripWidget() override = default;

    ChannelType channelType() const { return m_type; }

    void addApp(const QString& appName, uint32_t pid);
    void removeApp(const QString& appName);
    bool hasApp(const QString& appName) const;
    uint32_t getAppPid(const QString& appName) const;
    void clearApps();

    void updateVolumesFromEngine();

signals:
    void appReassigned(const QString& appName, ChannelType targetChannel);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    void setupUi();
    QWidget* createFaderColumn(bool isStream);

    ChannelType m_type;
    QString m_channelName;
    QString m_icon;
    QString m_accentColor;

    QSlider *m_monitorSlider = nullptr;
    QLabel *m_monitorValueLabel = nullptr;
    QPushButton *m_monitorMuteBtn = nullptr;

    QSlider *m_streamSlider = nullptr;
    QLabel *m_streamValueLabel = nullptr;
    QPushButton *m_streamMuteBtn = nullptr;

    QWidget *m_appsContainer = nullptr;
    QVBoxLayout *m_appsLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;

    std::unordered_map<std::string, AppBadgeWidget*> m_appBadges;
};

} // namespace AudioRouter

