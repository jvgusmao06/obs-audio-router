/**
 * @file channel-strip-widget.cpp
 * @brief Implementation of ChannelStripWidget handling vertical faders, independent mutes,
 *        VU meters, drag-and-drop targets, and dynamically expanding app badges.
 */

#include "channel-strip-widget.hpp"
#include "app-badge-widget.hpp"
#include "vu-meter-widget.hpp"
#include "localization-helper.hpp"
#include "core/channel-audio-engine.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QScrollArea>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <algorithm>

namespace AudioRouter {

ChannelStripWidget::ChannelStripWidget(ChannelType type, QWidget *parent)
    : QFrame(parent), m_type(type)
{
    if (type != ChannelType::MASTER) {
        setAcceptDrops(true);
    }

    switch (type) {
        case ChannelType::MASTER:
            m_channelName = "MASTER";
            m_icon = "🎛️";
            m_accentColor = "#3b82f6";
            break;
        case ChannelType::GAME:
            m_channelName = QString::fromStdString(loc("ChannelGame", "Game"));
            m_icon = "🎮";
            m_accentColor = "#00f0aa";
            break;
        case ChannelType::CHAT:
            m_channelName = QString::fromStdString(loc("ChannelChat", "Chat"));
            m_icon = "💬";
            m_accentColor = "#5865f2";
            break;
        case ChannelType::MEDIA:
            m_channelName = QString::fromStdString(loc("ChannelMedia", "Media"));
            m_icon = "🎵";
            m_accentColor = "#eb459e";
            break;
        case ChannelType::SYSTEM:
        default:
            m_channelName = QString::fromStdString(loc("ChannelSystem", "System"));
            m_icon = "⚙️";
            m_accentColor = "#faa61a";
            break;
    }

    setupUi();
    updateVolumesFromEngine();
}

void ChannelStripWidget::setupUi() {
    setMinimumWidth(115);
    setStyleSheet(
        "ChannelStripWidget {"
        "  background-color: #202225;"
        "  border: 1px solid #2f3136;"
        "  border-radius: 8px;"
        "}"
    );

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(6, 6, 6, 6);
    rootLayout->setSpacing(4);

    // 1. Cabeçalho do Canal (Ícone + Título)
    auto *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(4);

    auto *iconLabel = new QLabel(m_icon, this);
    iconLabel->setStyleSheet(QString("font-size: 13px; color: %1; border: none;").arg(m_accentColor));
    headerLayout->addWidget(iconLabel);

    auto *titleLabel = new QLabel(QString("<b>%1</b>").arg(m_channelName.toUpper()), this);
    titleLabel->setStyleSheet("font-size: 11px; color: #ffffff; border: none; letter-spacing: 0.5px;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    rootLayout->addLayout(headerLayout);

    // Linha divisória sutil
    auto *divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #2f3136; border: none; max-height: 1px;");
    rootLayout->addWidget(divider);

    // 2. Seção de Faders Duplos (Fone vs Stream) - Tamanho fixo ergonômico
    auto *fadersLayout = new QHBoxLayout();
    fadersLayout->setContentsMargins(0, 2, 0, 2);
    fadersLayout->setSpacing(6);

    fadersLayout->addWidget(createFaderColumn(false)); // Fone (Headphones)
    fadersLayout->addWidget(createFaderColumn(true));  // Stream (Live)

    rootLayout->addLayout(fadersLayout, 0);

    // 3. Seção Inferior: Apps Auto-Alocados (Oculto na coluna MASTER)
    if (m_type != ChannelType::MASTER) {
        auto *appsHeaderLayout = new QHBoxLayout();
        appsHeaderLayout->setContentsMargins(0, 2, 0, 0);
        auto *appsLabel = new QLabel("APPS", this);
        appsLabel->setStyleSheet("font-size: 10px; font-weight: bold; color: #8e9297; border: none;");
        appsHeaderLayout->addWidget(appsLabel);
        appsHeaderLayout->addStretch();
        rootLayout->addLayout(appsHeaderLayout);

        m_scrollArea = new QScrollArea(this);
        m_scrollArea->setWidgetResizable(true);
        m_scrollArea->setMinimumHeight(65);
        m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->setStyleSheet(
            "QScrollArea { background-color: #18191c; border: 1px solid #2f3136; border-radius: 6px; }"
            "QScrollBar:vertical { width: 4px; background: transparent; }"
            "QScrollBar::handle:vertical { background: #4f545c; border-radius: 2px; }"
        );

        m_appsContainer = new QWidget();
        m_appsContainer->setStyleSheet("background-color: transparent; border: none;");
        m_appsLayout = new QVBoxLayout(m_appsContainer);
        m_appsLayout->setContentsMargins(4, 4, 4, 4);
        m_appsLayout->setSpacing(4);
        m_appsLayout->addStretch();

        m_scrollArea->setWidget(m_appsContainer);
        rootLayout->addWidget(m_scrollArea, 1);
    } else {
        rootLayout->addStretch(1);
    }
}

QWidget* ChannelStripWidget::createFaderColumn(bool isStream) {
    auto *colWidget = new QWidget(this);
    colWidget->setStyleSheet("background: transparent; border: none;");
    auto *colLayout = new QVBoxLayout(colWidget);
    colLayout->setContentsMargins(0, 0, 0, 0);
    colLayout->setSpacing(3);

    // Ícone da Mixagem (🎧 ou 📡)
    QString iconStr = isStream ? "📡" : "🎧";
    QString subtitle = isStream ? QString::fromStdString(loc("VolumeStream", "Stream"))
                                : QString::fromStdString(loc("VolumeMonitor", "Headphones"));
    QString accent = isStream ? "#5865f2" : m_accentColor;

    auto *headerLabel = new QLabel(QString("%1 %2").arg(iconStr).arg(subtitle), colWidget);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet("font-size: 10px; color: #8e9297; font-weight: 600; border: none;");
    colLayout->addWidget(headerLabel);

    // Fader Vertical com VU Meter ao lado
    auto *faderRow = new QWidget(colWidget);
    auto *faderRowLayout = new QHBoxLayout(faderRow);
    faderRowLayout->setContentsMargins(0, 0, 0, 0);
    faderRowLayout->setSpacing(3);
    faderRowLayout->setAlignment(Qt::AlignCenter);

    auto *slider = new QSlider(Qt::Vertical, faderRow);
    slider->setRange(0, 100);
    slider->setFixedHeight(105);
    slider->setFixedWidth(22);
    slider->setStyleSheet(QString(
        "QSlider::groove:vertical {"
        "    background: #18191c;"
        "    width: 6px;"
        "    border-radius: 3px;"
        "    border: 1px solid #2f3136;"
        "}"
        "QSlider::handle:vertical {"
        "    background: #dcddde;"
        "    height: 12px;"
        "    width: 18px;"
        "    margin: 0 -6px;"
        "    border-radius: 2px;"
        "    border: 1px solid #72767d;"
        "}"
        "QSlider::handle:vertical:hover {"
        "    background: #ffffff;"
        "    border-color: %1;"
        "}"
    ).arg(accent));

    auto *vuMeter = new VuMeterWidget(faderRow);

    faderRowLayout->addWidget(slider);
    faderRowLayout->addWidget(vuMeter);

    colLayout->addWidget(faderRow, 0, Qt::AlignHCenter);

    // Rótulo numérico de porcentagem
    auto *valLabel = new QLabel("100%", colWidget);
    valLabel->setAlignment(Qt::AlignCenter);
    valLabel->setStyleSheet("font-size: 10px; color: #dcddde; font-weight: bold; border: none;");
    colLayout->addWidget(valLabel);

    // Botão de Mudo Compacto com Ícone (Tamanho fixo, não escalável)
    auto *muteBtn = new QPushButton(QString::fromUtf8("🔊"), colWidget);
    muteBtn->setCheckable(true);
    muteBtn->setFixedSize(26, 20);
    muteBtn->setCursor(Qt::PointingHandCursor);
    muteBtn->setToolTip(QString::fromStdString(loc("Mute", "Mute")));
    muteBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #2b2e38;"
        "  color: #dcddde;"
        "  border: 1px solid #3e4451;"
        "  border-radius: 3px;"
        "  font-size: 11px;"
        "  padding: 0px;"
        "}"
        "QPushButton:hover { background-color: #353945; color: #ffffff; }"
        "QPushButton:checked { background-color: #ef4444; color: #ffffff; border: 1px solid #dc2626; }"
    );
    colLayout->addWidget(muteBtn, 0, Qt::AlignHCenter);

    if (isStream) {
        m_streamSlider = slider;
        m_streamValueLabel = valLabel;
        m_streamMuteBtn = muteBtn;
        m_streamMeter = vuMeter;

        connect(slider, &QSlider::valueChanged, this, [this](int val) {
            m_streamValueLabel->setText(QString("%1%").arg(val));
            ChannelAudioEngine::instance().setStreamVolume(m_type, static_cast<float>(val) / 100.0f);
        });

        connect(muteBtn, &QPushButton::toggled, this, [this, muteBtn](bool checked) {
            muteBtn->setText(checked ? QString::fromUtf8("🔇") : QString::fromUtf8("🔊"));
            muteBtn->setToolTip(checked ? QString::fromStdString(loc("Unmute", "Unmute")) : QString::fromStdString(loc("Mute", "Mute")));
            ChannelAudioEngine::instance().setStreamMuted(m_type, checked);
        });
    } else {
        m_monitorSlider = slider;
        m_monitorValueLabel = valLabel;
        m_monitorMuteBtn = muteBtn;
        m_monitorMeter = vuMeter;

        connect(slider, &QSlider::valueChanged, this, [this](int val) {
            m_monitorValueLabel->setText(QString("%1%").arg(val));
            ChannelAudioEngine::instance().setMonitorVolume(m_type, static_cast<float>(val) / 100.0f);
        });

        connect(muteBtn, &QPushButton::toggled, this, [this, muteBtn](bool checked) {
            muteBtn->setText(checked ? QString::fromUtf8("🔇") : QString::fromUtf8("🔊"));
            muteBtn->setToolTip(checked ? QString::fromStdString(loc("Unmute", "Unmute")) : QString::fromStdString(loc("Mute", "Mute")));
            ChannelAudioEngine::instance().setMonitorMuted(m_type, checked);
        });
    }

    return colWidget;
}

void ChannelStripWidget::updatePeak(float peak) {
    if (m_monitorMeter) m_monitorMeter->setLevel(peak);
    if (m_streamMeter) m_streamMeter->setLevel(peak);
}

void ChannelStripWidget::updateVolumesFromEngine() {
    float streamVol = ChannelAudioEngine::instance().getStreamVolume(m_type);
    float monitorVol = ChannelAudioEngine::instance().getMonitorVolume(m_type);
    bool streamMuted = ChannelAudioEngine::instance().isStreamMuted(m_type);
    bool monitorMuted = ChannelAudioEngine::instance().isMonitorMuted(m_type);

    if (m_streamSlider) {
        m_streamSlider->blockSignals(true);
        int val = static_cast<int>(streamVol * 100.0f);
        m_streamSlider->setValue(val);
        m_streamValueLabel->setText(QString("%1%").arg(val));
        m_streamSlider->blockSignals(false);
    }

    if (m_monitorSlider) {
        m_monitorSlider->blockSignals(true);
        int val = static_cast<int>(monitorVol * 100.0f);
        m_monitorSlider->setValue(val);
        m_monitorValueLabel->setText(QString("%1%").arg(val));
        m_monitorSlider->blockSignals(false);
    }

    if (m_streamMuteBtn) {
        m_streamMuteBtn->blockSignals(true);
        m_streamMuteBtn->setChecked(streamMuted);
        m_streamMuteBtn->setText(streamMuted ? QString::fromUtf8("🔇") : QString::fromUtf8("🔊"));
        m_streamMuteBtn->setToolTip(streamMuted ? QString::fromStdString(loc("Unmute", "Unmute")) : QString::fromStdString(loc("Mute", "Mute")));
        m_streamMuteBtn->blockSignals(false);
    }

    if (m_monitorMuteBtn) {
        m_monitorMuteBtn->blockSignals(true);
        m_monitorMuteBtn->setChecked(monitorMuted);
        m_monitorMuteBtn->setText(monitorMuted ? QString::fromUtf8("🔇") : QString::fromUtf8("🔊"));
        m_monitorMuteBtn->setToolTip(monitorMuted ? QString::fromStdString(loc("Unmute", "Unmute")) : QString::fromStdString(loc("Mute", "Mute")));
        m_monitorMuteBtn->blockSignals(false);
    }
}

void ChannelStripWidget::addApp(const QString& appName, uint32_t pid) {
    std::string key = appName.toStdString();
    if (m_appBadges.find(key) != m_appBadges.end()) return;

    auto *badge = new AppBadgeWidget(appName, pid, m_appsContainer);
    connect(badge, &AppBadgeWidget::reassignRequested, this, [this](const QString& app, ChannelType target) {
        emit appReassigned(app, target);
    });

    m_appBadges[key] = badge;
    m_appsLayout->insertWidget(m_appsLayout->count() - 1, badge);
}

void ChannelStripWidget::removeApp(const QString& appName) {
    std::string key = appName.toStdString();
    auto it = m_appBadges.find(key);
    if (it != m_appBadges.end()) {
        m_appsLayout->removeWidget(it->second);
        it->second->deleteLater();
        m_appBadges.erase(it);
    }
}

bool ChannelStripWidget::hasApp(const QString& appName) const {
    return m_appBadges.find(appName.toStdString()) != m_appBadges.end();
}

uint32_t ChannelStripWidget::getAppPid(const QString& appName) const {
    auto it = m_appBadges.find(appName.toStdString());
    return (it != m_appBadges.end()) ? it->second->processId() : 0;
}

void ChannelStripWidget::clearApps() {
    for (auto& [key, badge] : m_appBadges) {
        m_appsLayout->removeWidget(badge);
        badge->deleteLater();
    }
    m_appBadges.clear();
}

void ChannelStripWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (m_type == ChannelType::MASTER) return;
    if (event->mimeData()->hasFormat("application/x-audio-router-app")) {
        event->acceptProposedAction();
        setStyleSheet(
            QString("ChannelStripWidget { background-color: #26282c; border: 2px dashed %1; border-radius: 8px; }").arg(m_accentColor)
        );
    }
}

void ChannelStripWidget::dragMoveEvent(QDragMoveEvent *event) {
    if (m_type == ChannelType::MASTER) return;
    if (event->mimeData()->hasFormat("application/x-audio-router-app")) {
        event->acceptProposedAction();
    }
}

void ChannelStripWidget::dragLeaveEvent(QDragLeaveEvent *event) {
    setStyleSheet(
        "ChannelStripWidget { background-color: #202225; border: 1px solid #2f3136; border-radius: 8px; }"
    );
    event->accept();
}

void ChannelStripWidget::dropEvent(QDropEvent *event) {
    setStyleSheet(
        "ChannelStripWidget { background-color: #202225; border: 1px solid #2f3136; border-radius: 8px; }"
    );

    if (m_type == ChannelType::MASTER) return;

    if (event->mimeData()->hasFormat("application/x-audio-router-app")) {
        QString appName = QString::fromUtf8(event->mimeData()->data("application/x-audio-router-app"));
        event->acceptProposedAction();
        emit appReassigned(appName, m_type);
    }
}

} // namespace AudioRouter
