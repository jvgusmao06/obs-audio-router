#pragma once

#include <QLabel>
#include <QPoint>

namespace AudioRouter {

class AppBadgeWidget : public QLabel {
    Q_OBJECT

public:
    explicit AppBadgeWidget(const QString& appName, uint32_t pid, QWidget *parent = nullptr);

    QString appName() const { return m_appName; }
    uint32_t processId() const { return m_pid; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QString m_appName;
    uint32_t m_pid;
    QPoint m_dragStartPosition;
};

} // namespace AudioRouter

