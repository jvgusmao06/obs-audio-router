#include "app-badge-widget.hpp"
#include "localization-helper.hpp"
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QDrag>
#include <QMimeData>
#include <QApplication>

namespace AudioRouter {

AppBadgeWidget::AppBadgeWidget(const QString& appName, uint32_t pid, QWidget *parent)
    : QLabel(appName, parent), m_appName(appName), m_pid(pid)
{
    setCursor(Qt::PointingHandCursor);
    setAlignment(Qt::AlignCenter);
    setStyleSheet(
        "QLabel {"
        "  background-color: #202225;"
        "  color: #dcddde;"
        "  border: 1px solid #2f3136;"
        "  border-radius: 4px;"
        "  padding: 4px 6px;"
        "  font-weight: 500;"
        "  font-size: 11px;"
        "}"
        "QLabel:hover {"
        "  background-color: #2f3136;"
        "  border-color: #5865f2;"
        "  color: #ffffff;"
        "}"
    );
    QString dragHint = QString::fromStdString(loc("DragDropHint", "Drag or right-click to change channel"));
    setToolTip(QString("%1 (PID: %2)\n%3").arg(appName).arg(pid).arg(dragHint));
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

void AppBadgeWidget::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background-color: #18191c; border: 1px solid #2f3136; border-radius: 6px; padding: 4px; }"
        "QMenu::item { color: #dcddde; padding: 6px 14px; border-radius: 4px; }"
        "QMenu::item:selected { background-color: #5865f2; color: #ffffff; }"
    );

    QString moveTo = QString::fromStdString(loc("MoveTo", "Move to"));
    QString gameName = QString::fromStdString(loc("ChannelGame", "Game"));
    QString chatName = QString::fromStdString(loc("ChannelChat", "Chat"));
    QString mediaName = QString::fromStdString(loc("ChannelMedia", "Media"));
    QString sysName = QString::fromStdString(loc("ChannelSystem", "System"));

    auto *gameAction = menu.addAction(QString("🎮 %1 %2").arg(moveTo, gameName));
    auto *chatAction = menu.addAction(QString("💬 %1 %2").arg(moveTo, chatName));
    auto *mediaAction = menu.addAction(QString("🎵 %1 %2").arg(moveTo, mediaName));
    auto *sysAction = menu.addAction(QString("⚙️ %1 %2").arg(moveTo, sysName));

    QAction *selected = menu.exec(event->globalPos());
    if (selected == gameAction) {
        emit reassignRequested(m_appName, ChannelType::GAME);
    } else if (selected == chatAction) {
        emit reassignRequested(m_appName, ChannelType::CHAT);
    } else if (selected == mediaAction) {
        emit reassignRequested(m_appName, ChannelType::MEDIA);
    } else if (selected == sysAction) {
        emit reassignRequested(m_appName, ChannelType::SYSTEM);
    }
}

} // namespace AudioRouter
