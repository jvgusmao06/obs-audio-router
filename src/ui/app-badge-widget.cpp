#include "app-badge-widget.hpp"
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>

namespace AudioRouter {

AppBadgeWidget::AppBadgeWidget(const QString& appName, uint32_t pid, QWidget *parent)
    : QLabel(appName, parent), m_appName(appName), m_pid(pid)
{
    setCursor(Qt::PointingHandCursor);
    setStyleSheet(
        "QLabel {"
        "  background-color: #2f3136;"
        "  color: #00ffaa;"
        "  border: 1px solid #4f545c;"
        "  border-radius: 4px;"
        "  padding: 4px 10px;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "}"
        "QLabel:hover {"
        "  background-color: #40444b;"
        "  border-color: #7289da;"
        "}"
    );
    setToolTip(QString("PID: %1\nDrag to assign to a channel").arg(pid));
}

void AppBadgeWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }
    QLabel::mousePressEvent(event);
}

void AppBadgeWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton)) return;

    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    auto *drag = new QDrag(this);
    auto *mimeData = new QMimeData();

    mimeData->setText(m_appName);
    mimeData->setData("application/x-audio-router-app", m_appName.toUtf8());

    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}

} // namespace AudioRouter
