#include "audio-router-dock.hpp"
#include "app-badge-widget.hpp"
#include "localization-helper.hpp"
#include "core/channel-audio-engine.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QFrame>
#include <QMetaObject>

namespace AudioRouter {

AudioRouterDock::AudioRouterDock(QWidget *parent)
    : QDockWidget(parent),
      m_rules(std::make_unique<RoutingRules>()),
      m_sessionWatcher(std::make_unique<AudioSessionWatcher>())
{
    setWindowTitle(QString::fromStdString(loc("DockTitle", "Audio Router (Sonar Style)")));
    setObjectName("AudioRouterDock");

    setupUi();

    // Conecta o listener do WASAPI para atualizar a UI quando novos apps emitirem áudio
    m_sessionWatcher->setOnSessionCreated([this](const AudioSessionInfo& session) {
        if (!session.processName.empty()) {
            QMetaObject::invokeMethod(this, "onNewSessionDetected",
                Qt::QueuedConnection,
                Q_ARG(QString, QString::fromStdString(session.processName)),
                Q_ARG(uint32_t, session.processId));
        }
    });

    m_sessionWatcher->start();
    refreshActiveApplications();
}

AudioRouterDock::~AudioRouterDock() {
    if (m_sessionWatcher) {
        m_sessionWatcher->stop();
    }
}

void AudioRouterDock::setupUi() {
    auto *mainContainer = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(mainContainer);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(10);

    // 1. Barra Superior: Aplicativos Ativos Detectados
    auto *activeAppsHeader = new QLabel(
        QString::fromStdString(loc("ActiveAppsTitle", "Active Applications (Drag to assign):")), this);
    activeAppsHeader->setStyleSheet("font-weight: bold; color: #b9bbbe; font-size: 12px;");
    rootLayout->addWidget(activeAppsHeader);

    auto *activeAppsFrame = new QFrame(this);
    activeAppsFrame->setStyleSheet("background-color: #202225; border: 1px solid #2f3136; border-radius: 6px;");
    m_activeAppsLayout = new QHBoxLayout(activeAppsFrame);
    m_activeAppsLayout->setContentsMargins(6, 6, 6, 6);
    m_activeAppsLayout->setSpacing(8);
    m_activeAppsLayout->addStretch();

    rootLayout->addWidget(activeAppsFrame);

    // 2. Colunas de Canais (Jogo, Chat, Mídia, Sistema)
    auto *channelsContainer = new QWidget(this);
    m_channelsLayout = new QHBoxLayout(channelsContainer);
    m_channelsLayout->setContentsMargins(0, 0, 0, 0);
    m_channelsLayout->setSpacing(8);

    m_channelsLayout->addWidget(createChannelColumn("ChannelGame", ChannelType::GAME));
    m_channelsLayout->addWidget(createChannelColumn("ChannelChat", ChannelType::CHAT));
    m_channelsLayout->addWidget(createChannelColumn("ChannelMedia", ChannelType::MEDIA));
    m_channelsLayout->addWidget(createChannelColumn("ChannelSystem", ChannelType::SYSTEM));

    rootLayout->addWidget(channelsContainer);
    setWidget(mainContainer);
}

QWidget* AudioRouterDock::createChannelColumn(const QString& titleKey, ChannelType type) {
    auto *colFrame = new QFrame(this);
    colFrame->setStyleSheet("QFrame { background-color: #2f3136; border: 1px solid #202225; border-radius: 8px; }");

    auto *colLayout = new QVBoxLayout(colFrame);
    colLayout->setContentsMargins(8, 8, 8, 8);
    colLayout->setSpacing(6);

    // Título do Canal traduzido
    std::string defaultTitle = (type == ChannelType::GAME) ? "Game" :
                               (type == ChannelType::CHAT) ? "Chat" :
                               (type == ChannelType::MEDIA) ? "Media" : "System";
    QString title = QString::fromStdString(loc(titleKey.toStdString(), defaultTitle));

    auto *titleLabel = new QLabel(QString("<b>%1</b>").arg(title), colFrame);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("border: none; color: #ffffff; font-size: 13px; text-transform: uppercase;");
    colLayout->addWidget(titleLabel);

    // Zona receptora de Drag & Drop
    auto *dropZone = new ChannelDropZone(type, colFrame);
    dropZone->setOnAppDropped([this](const QString& appName, ChannelType targetChannel) {
        onAppAssignedToChannel(appName, targetChannel);
    });
    m_dropZones[type] = dropZone;
    colLayout->addWidget(dropZone);

    // Sliders de Volume Duplo (Stream vs Fone)
    auto *slidersLayout = new QHBoxLayout();

    // Slider Stream
    auto *streamBox = new QVBoxLayout();
    auto *streamLabel = new QLabel(QString::fromStdString(loc("VolumeStream", "Stream")), colFrame);
    streamLabel->setStyleSheet("border: none; font-size: 10px; color: #b9bbbe;");
    streamLabel->setAlignment(Qt::AlignCenter);
    auto *streamSlider = new QSlider(Qt::Vertical, colFrame);
    streamSlider->setRange(0, 100);
    streamSlider->setValue(static_cast<int>(ChannelAudioEngine::instance().getStreamVolume(type) * 100));
    streamSlider->setStyleSheet("QSlider::groove:vertical { background: #202225; width: 6px; border-radius: 3px; } QSlider::handle:vertical { background: #5865f2; height: 12px; margin: 0 -4px; border-radius: 6px; }");
    connect(streamSlider, &QSlider::valueChanged, [type](int val) {
        ChannelAudioEngine::instance().setStreamVolume(type, static_cast<float>(val) / 100.0f);
    });
    streamBox->addWidget(streamSlider, 0, Qt::AlignHCenter);
    streamBox->addWidget(streamLabel);
    slidersLayout->addLayout(streamBox);

    // Slider Fone (Monitor)
    auto *monitorBox = new QVBoxLayout();
    auto *monitorLabel = new QLabel(QString::fromStdString(loc("VolumeMonitor", "Headphones")), colFrame);
    monitorLabel->setStyleSheet("border: none; font-size: 10px; color: #b9bbbe;");
    monitorLabel->setAlignment(Qt::AlignCenter);
    auto *monitorSlider = new QSlider(Qt::Vertical, colFrame);
    monitorSlider->setRange(0, 100);
    monitorSlider->setValue(static_cast<int>(ChannelAudioEngine::instance().getMonitorVolume(type) * 100));
    monitorSlider->setStyleSheet("QSlider::groove:vertical { background: #202225; width: 6px; border-radius: 3px; } QSlider::handle:vertical { background: #00ffaa; height: 12px; margin: 0 -4px; border-radius: 6px; }");
    connect(monitorSlider, &QSlider::valueChanged, [type](int val) {
        ChannelAudioEngine::instance().setMonitorVolume(type, static_cast<float>(val) / 100.0f);
    });
    monitorBox->addWidget(monitorSlider, 0, Qt::AlignHCenter);
    monitorBox->addWidget(monitorLabel);
    slidersLayout->addLayout(monitorBox);

    colLayout->addLayout(slidersLayout);

    // Botão Mudo
    auto *muteBtn = new QPushButton(QString::fromStdString(loc("Mute", "Mute")), colFrame);
    muteBtn->setCheckable(true);
    muteBtn->setChecked(ChannelAudioEngine::instance().isMuted(type));
    muteBtn->setStyleSheet(
        "QPushButton { background-color: #36393f; color: #dcddde; border: none; padding: 4px; border-radius: 4px; font-weight: bold; } "
        "QPushButton:hover { background-color: #4f545c; } "
        "QPushButton:checked { background-color: #ed4245; color: white; }"
    );
    connect(muteBtn, &QPushButton::toggled, [type](bool checked) {
        ChannelAudioEngine::instance().setMuted(type, checked);
    });
    colLayout->addWidget(muteBtn);

    return colFrame;
}

void AudioRouterDock::refreshActiveApplications() {
    auto sessions = m_sessionWatcher->getActiveSessions();
    for (const auto& s : sessions) {
        if (!s.processName.empty()) {
            onNewSessionDetected(QString::fromStdString(s.processName), s.processId);
        }
    }
}

void AudioRouterDock::onNewSessionDetected(const QString& appName, uint32_t pid) {
    std::string appStd = appName.toStdString();
    if (m_knownApps[appStd]) return;
    m_knownApps[appStd] = true;

    // 1. Adiciona badge na barra superior
    auto *badge = new AppBadgeWidget(appName, pid, this);
    m_activeAppsLayout->insertWidget(m_activeAppsLayout->count() - 1, badge);

    // 2. Resolve automaticamente para qual canal ele deve ir de início
    ChannelType assignedChannel = m_rules->resolveChannel(appStd);
    auto it = m_dropZones.find(assignedChannel);
    if (it != m_dropZones.end()) {
        it->second->addAssignedApp(appName);
    }
}

void AudioRouterDock::onAppAssignedToChannel(const QString& appName, ChannelType channel) {
    std::string appStd = appName.toStdString();
    m_rules->setCustomRule(appStd, channel);
    ChannelAudioEngine::instance().assignProcessToChannel(appStd, channel);
}

} // namespace AudioRouter
