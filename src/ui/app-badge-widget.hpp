/**
 * @file app-badge-widget.hpp
 * @brief Header for AppBadgeWidget, a draggable interactive chip displaying
 *        an active process with context menu channel reassignment.
 */

#pragma once

#include <QLabel>
#include <QPoint>
#include "core/routing-rules.hpp"

namespace AudioRouter {

class AppBadgeWidget : public QLabel {
    Q_OBJECT

public:
    explicit AppBadgeWidget(const QString& appName, uint32_t pid, QWidget *parent = nullptr);

    QString appName() const { return m_appName; }
    uint32_t processId() const { return m_pid; }

signals:
    void reassignRequested(const QString& appName, ChannelType newChannel);
    void makeIgnoredRequested(const QString& appName);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QString m_appName;
    uint32_t m_pid;
    QPoint m_dragStartPosition;
};

} // namespace AudioRouter
