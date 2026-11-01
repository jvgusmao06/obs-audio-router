#pragma once

#include <QFrame>
#include <QVBoxLayout>
#include <QLabel>
#include <functional>
#include "core/routing-rules.hpp"

namespace AudioRouter {

class ChannelDropZone : public QFrame {
    Q_OBJECT

public:
    using AppDroppedCallback = std::function<void(const QString& appName, ChannelType targetChannel)>;

    explicit ChannelDropZone(ChannelType channelType, QWidget *parent = nullptr);

    void setOnAppDropped(AppDroppedCallback callback);
    void addAssignedApp(const QString& appName);
    void removeAssignedApp(const QString& appName);
    void clearApps();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    ChannelType m_channelType;
    QVBoxLayout *m_appsLayout = nullptr;
    QLabel *m_emptyHint = nullptr;
    AppDroppedCallback m_onAppDropped;
};

} // namespace AudioRouter

