#include "channel-drop-zone.hpp"
#include "localization-helper.hpp"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPushButton>
#include <QHBoxLayout>

namespace AudioRouter {

ChannelDropZone::ChannelDropZone(ChannelType channelType, QWidget *parent)
    : QFrame(parent), m_channelType(channelType)
{
    setAcceptDrops(true);
    setStyleSheet(
        "QFrame {"
        "  background-color: #18191c;"
        "  border: 1px dashed #40444b;"
        "  border-radius: 6px;"
        "  min-height: 80px;"
        "}"
    );

    m_appsLayout = new QVBoxLayout(this);
    m_appsLayout->setContentsMargins(6, 6, 6, 6);
    m_appsLayout->setSpacing(4);

    m_emptyHint = new QLabel(QString::fromStdString(loc("DropHere", "Drop apps here")), this);
    m_emptyHint->setStyleSheet("border: none; color: #72767d; font-size: 11px;");
    m_emptyHint->setAlignment(Qt::AlignCenter);
    m_appsLayout->addWidget(m_emptyHint);
}

void ChannelDropZone::setOnAppDropped(AppDroppedCallback callback) {
    m_onAppDropped = callback;
}

void ChannelDropZone::addAssignedApp(const QString& appName) {
    if (m_emptyHint) {
        m_emptyHint->hide();
    }

    // Create item widget with remove 'X' button
    auto *itemWidget = new QWidget(this);
    itemWidget->setStyleSheet("background-color: #2f3136; border: 1px solid #36393f; border-radius: 4px; padding: 2px;");
    auto *itemLayout = new QHBoxLayout(itemWidget);
    itemLayout->setContentsMargins(4, 2, 4, 2);

    auto *nameLabel = new QLabel(appName, itemWidget);
    nameLabel->setStyleSheet("border: none; color: #dcddde; font-size: 11px; font-weight: bold;");
    itemLayout->addWidget(nameLabel);
    itemLayout->addStretch();

    auto *removeBtn = new QPushButton("✖", itemWidget);
    removeBtn->setFixedSize(16, 16);
    removeBtn->setCursor(Qt::PointingHandCursor);
    removeBtn->setStyleSheet("QPushButton { border: none; color: #ed4245; font-size: 10px; background: transparent; } QPushButton:hover { color: #ff7875; }");

    connect(removeBtn, &QPushButton::clicked, [this, itemWidget, appName]() {
        itemWidget->deleteLater();
        if (m_onAppDropped) {
            // Revert back to SYSTEM fallback
            m_onAppDropped(appName, ChannelType::SYSTEM);
        }
    });

    itemLayout->addWidget(removeBtn);
    m_appsLayout->addWidget(itemWidget);
}

void ChannelDropZone::clearApps() {
    QLayoutItem *item;
    while ((item = m_appsLayout->takeAt(0)) != nullptr) {
        if (item->widget() && item->widget() != m_emptyHint) {
            delete item->widget();
        }
        delete item;
    }
    if (m_emptyHint) {
        m_appsLayout->addWidget(m_emptyHint);
        m_emptyHint->show();
    }
}

void ChannelDropZone::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("application/x-audio-router-app") || event->mimeData()->hasText()) {
        setStyleSheet(
            "QFrame {"
            "  background-color: #202225;"
            "  border: 2px solid #5865f2;"
            "  border-radius: 6px;"
            "  min-height: 80px;"
            "}"
        );
        event->acceptProposedAction();
    }
}

void ChannelDropZone::dragLeaveEvent(QDragLeaveEvent * /*event*/) {
    setStyleSheet(
        "QFrame {"
        "  background-color: #18191c;"
        "  border: 1px dashed #40444b;"
        "  border-radius: 6px;"
        "  min-height: 80px;"
        "}"
    );
}

void ChannelDropZone::dropEvent(QDropEvent *event) {
    QString appName = event->mimeData()->text();
    if (appName.isEmpty() && event->mimeData()->hasFormat("application/x-audio-router-app")) {
        appName = QString::fromUtf8(event->mimeData()->data("application/x-audio-router-app"));
    }

    dragLeaveEvent(nullptr);

    if (!appName.isEmpty()) {
        addAssignedApp(appName);
        if (m_onAppDropped) {
            m_onAppDropped(appName, m_channelType);
        }
        event->acceptProposedAction();
    }
}

} // namespace AudioRouter
