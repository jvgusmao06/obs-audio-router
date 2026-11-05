/**
 * @file audio-router-dock.cpp
 * @brief Implementation of AudioRouterDock orchestrating the UI layout, WASAPI session
 *        lifecycle events, rule persistence, ignored apps manager, and mixer channel strips.
 */

#include "audio-router-dock.hpp"
#include "channel-strip-widget.hpp"
#include "chatmix-slider.hpp"
#include "ignored-apps-dialog.hpp"
#include "localization-helper.hpp"
#include "core/channel-audio-engine.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolTip>
#include <QFrame>
#include <QTimer>
#include <QMetaObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <unordered_set>

namespace AudioRouter {

static QString getRulesConfigPath() {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (configDir.isEmpty()) {
        configDir = "data";
    } else {
        configDir += "/obs-audio-router";
    }
    QDir().mkpath(configDir);
    return configDir + "/rules.json";
}

AudioRouterDock::AudioRouterDock(QWidget *parent)
    : QWidget(parent),
      m_rules(std::make_unique<RoutingRules>()),
      m_sessionWatcher(std::make_unique<AudioSessionWatcher>())
{
    setObjectName("AudioRouterDock");

    // Carrega regras salvas previamente pelo usuário
    loadRulesFromDisk();

    // Vincula o SessionWatcher à engine para refletir volumes no Windows WASAPI
    ChannelAudioEngine::instance().setSessionWatcher(m_sessionWatcher.get());

    setupUi();

    // Timer de 30 FPS para atualizar os VU meters em tempo real
    m_meterTimer = new QTimer(this);
    connect(m_meterTimer, &QTimer::timeout, this, &AudioRouterDock::updateVuMeters);
    m_meterTimer->start(33);

    // Timer de 1s para sincronização e limpeza de aplicativos fechados
    m_sessionSyncTimer = new QTimer(this);
    connect(m_sessionSyncTimer, &QTimer::timeout, this, &AudioRouterDock::refreshActiveApplications);
    m_sessionSyncTimer->start(1000);

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

void AudioRouterDock::shutdown() {
    if (m_meterTimer) {
        m_meterTimer->stop();
    }
    if (m_sessionSyncTimer) {
        m_sessionSyncTimer->stop();
    }

    // Restaura o volume no Windows WASAPI para todos os processos gerenciados
    if (m_sessionWatcher) {
        for (auto type : {ChannelType::GAME, ChannelType::CHAT, ChannelType::MEDIA, ChannelType::SYSTEM}) {
            const auto* state = ChannelAudioEngine::instance().getChannelState(type);
            if (state) {
                for (const auto& proc : state->assignedProcesses) {
                    m_sessionWatcher->setProcessVolume(proc, 1.0f);
                    m_sessionWatcher->setProcessMute(proc, false);
                }
            }
        }
    }

    ChannelAudioEngine::instance().setSessionWatcher(nullptr);

    if (m_sessionWatcher) {
        m_sessionWatcher->stop();
    }

    ChannelAudioEngine::instance().shutdownChannels();
}

AudioRouterDock::~AudioRouterDock() {
    shutdown();
}

void AudioRouterDock::updateVuMeters() {
    for (auto& [type, strip] : m_channelStrips) {
        float peak = ChannelAudioEngine::instance().getChannelPeak(type);
        strip->updatePeak(peak);
    }
}

void AudioRouterDock::updateIgnoredBtnText() {
    if (!m_ignoredAppsBtn) return;
    auto ignoredList = m_rules->getIgnoredApps();
    QString btnLabel = QString::fromUtf8("🚫 ") + QString::fromStdString(loc("IgnoredApps", "Ignored Apps"));
    if (!ignoredList.empty()) {
        btnLabel += QString(" (%1)").arg(ignoredList.size());
    }
    m_ignoredAppsBtn->setText(btnLabel);
}

void AudioRouterDock::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(6, 6, 6, 6);
    rootLayout->setSpacing(6);

    // 1. Barra Superior: Logo, ChatMix Slider centralizado, Apps Ignorados e botão Ouvir Stream
    auto *topBarLayout = new QHBoxLayout();
    topBarLayout->setContentsMargins(2, 0, 2, 0);
    topBarLayout->setSpacing(8);

    auto *logoLabel = new QLabel("<b>AUDIO ROUTER</b>", this);
    logoLabel->setStyleSheet("color: #ffffff; font-size: 12px; letter-spacing: 1px; font-weight: bold; border: none;");
    topBarLayout->addWidget(logoLabel);

    auto *infoBtn = new QPushButton(QString::fromUtf8("ℹ"), this);
    infoBtn->setToolTip(QString::fromStdString(loc("DesktopAudioTip")));
    infoBtn->setFixedSize(18, 18);
    infoBtn->setCursor(Qt::PointingHandCursor);
    infoBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #717888;"
        "  border: 1px solid #3e4451;"
        "  border-radius: 9px;"
        "  font-size: 11px;"
        "  font-weight: bold;"
        "  padding: 0px;"
        "}"
        "QPushButton:hover {"
        "  color: #60a5fa;"
        "  border-color: #60a5fa;"
        "  background-color: #1e222b;"
        "}"
        "QPushButton:focus { outline: none; }"
    );
    connect(infoBtn, &QPushButton::clicked, [infoBtn]() {
        QToolTip::showText(infoBtn->mapToGlobal(QPoint(0, infoBtn->height())), infoBtn->toolTip());
    });
    topBarLayout->addWidget(infoBtn);

    topBarLayout->addStretch();

    m_chatMixSlider = new ChatMixSlider(this);
    connect(m_chatMixSlider, &ChatMixSlider::chatMixChanged, [](float factor) {
        ChannelAudioEngine::instance().setChatMix(factor);
    });
    topBarLayout->addWidget(m_chatMixSlider);

    topBarLayout->addStretch();

    // Botão de Apps Ignorados
    m_ignoredAppsBtn = new QPushButton(this);
    m_ignoredAppsBtn->setCursor(Qt::PointingHandCursor);
    m_ignoredAppsBtn->setFixedHeight(26);
    m_ignoredAppsBtn->setToolTip(QString::fromStdString(loc("IgnoredAppsSubtitle", "Applications in this list do not appear in Router and output directly to Windows without volume alterations.")));
    m_ignoredAppsBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #2b2e38;"
        "  color: #a0a6b2;"
        "  border: 1px solid #3e4451;"
        "  border-radius: 4px;"
        "  padding: 3px 8px;"
        "  font-size: 11px;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover { background-color: #353945; color: #ffffff; }"
    );
    connect(m_ignoredAppsBtn, &QPushButton::clicked, this, &AudioRouterDock::showIgnoredAppsDialog);
    topBarLayout->addWidget(m_ignoredAppsBtn);
    updateIgnoredBtnText();

    // Botão Ouvir Stream (Headphones preview)
    m_listenStreamBtn = new QPushButton(this);
    m_listenStreamBtn->setCheckable(true);
    m_listenStreamBtn->setCursor(Qt::PointingHandCursor);
    m_listenStreamBtn->setFixedHeight(26);
    m_listenStreamBtn->setToolTip(QString::fromStdString(loc("MonitorStreamTip", "Escute nos seus fones o áudio exatamente como está saindo para a stream")));

    auto updateListenBtnStyle = [this](bool checked) {
        if (checked) {
            m_listenStreamBtn->setText(QString::fromUtf8("🎧 ") + QString::fromStdString(loc("MonitoringStream", "Ouvindo Stream")));
            m_listenStreamBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #3b82f6;"
                "  color: #ffffff;"
                "  font-weight: bold;"
                "  border: 1px solid #60a5fa;"
                "  border-radius: 4px;"
                "  padding: 3px 8px;"
                "  font-size: 11px;"
                "}"
                "QPushButton:hover { background-color: #2563eb; }"
            );
        } else {
            m_listenStreamBtn->setText(QString::fromUtf8("👂 ") + QString::fromStdString(loc("MonitorStream", "Ouvir Stream")));
            m_listenStreamBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #2b2e38;"
                "  color: #a0a6b2;"
                "  border: 1px solid #3e4451;"
                "  border-radius: 4px;"
                "  padding: 3px 8px;"
                "  font-size: 11px;"
                "}"
                "QPushButton:hover { background-color: #353945; color: #ffffff; }"
            );
        }
    };

    updateListenBtnStyle(false);

    connect(m_listenStreamBtn, &QPushButton::toggled, this, [this, updateListenBtnStyle](bool checked) {
        ChannelAudioEngine::instance().setMonitoringStream(checked);
        updateListenBtnStyle(checked);
    });

    topBarLayout->addWidget(m_listenStreamBtn);

    rootLayout->addLayout(topBarLayout);

    // 2. Colunas de Canais Mixer (MASTER | GAME, CHAT, MEDIA, SYSTEM)
    auto *channelsContainer = new QWidget(this);
    m_channelsLayout = new QHBoxLayout(channelsContainer);
    m_channelsLayout->setContentsMargins(0, 0, 0, 0);
    m_channelsLayout->setSpacing(6);

    auto addChannel = [this](ChannelType type) {
        auto *strip = new ChannelStripWidget(type, this);
        connect(strip, &ChannelStripWidget::appReassigned, this, &AudioRouterDock::onAppAssignedToChannel);
        connect(strip, &ChannelStripWidget::appMadeIgnored, this, &AudioRouterDock::onAppMadeIgnored);
        m_channelStrips[type] = strip;
        m_channelsLayout->addWidget(strip);
    };

    // Coluna MASTER à extrema esquerda
    addChannel(ChannelType::MASTER);

    // Linha divisória sutil entre Master e subcanais
    auto *masterDivider = new QFrame(channelsContainer);
    masterDivider->setFrameShape(QFrame::VLine);
    masterDivider->setStyleSheet("background-color: #2f3136; border: none; max-width: 1px;");
    m_channelsLayout->addWidget(masterDivider);

    // Canais temáticos
    addChannel(ChannelType::GAME);
    addChannel(ChannelType::CHAT);
    addChannel(ChannelType::MEDIA);
    addChannel(ChannelType::SYSTEM);

    rootLayout->addWidget(channelsContainer, 1);
}

void AudioRouterDock::loadRulesFromDisk() {
    QFile file(getRulesConfigPath());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray data = file.readAll();
        m_rules->deserializeFromJson(data.toStdString());
        file.close();
    }
}

void AudioRouterDock::saveRulesToDisk() {
    QFile file(getRulesConfigPath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        std::string json = m_rules->serializeToJson();
        file.write(json.data(), json.size());
        file.close();
    }
}

void AudioRouterDock::refreshActiveApplications() {
    auto sessions = m_sessionWatcher->getActiveSessions();
    std::unordered_set<std::string> activeApps;

    for (const auto& s : sessions) {
        if (!s.processName.empty()) {
            activeApps.insert(s.processName);
            onNewSessionDetected(QString::fromStdString(s.processName), s.processId);
        }
    }

    // Remove do mixer aplicativos que foram fechados no Windows
    for (auto& [type, strip] : m_channelStrips) {
        auto displayedApps = strip->getAppNames();
        for (const auto& appName : displayedApps) {
            if (activeApps.find(appName) == activeApps.end()) {
                strip->removeApp(QString::fromStdString(appName));
                ChannelAudioEngine::instance().unassignProcess(appName);
            }
        }
    }
}

void AudioRouterDock::onNewSessionDetected(const QString& appName, uint32_t pid) {
    std::string appStd = appName.toStdString();

    // 0. Se o aplicativo estiver na lista de ignorados, não exibir nem interferir no áudio
    if (m_rules->isIgnored(appStd)) {
        return;
    }

    // 1. Resolve para qual canal o aplicativo deve ir (regra customizada ou padrão)
    ChannelType assignedChannel = m_rules->resolveChannel(appStd);

    // 2. Insere na seção de Apps da coluna correspondente (se ainda não adicionado visualmente)
    auto it = m_channelStrips.find(assignedChannel);
    if (it != m_channelStrips.end() && !it->second->hasApp(appName)) {
        it->second->addApp(appName, pid);
    }

    // 3. SEMPRE registra na engine e aplica volume/mute na nova sessão WASAPI
    ChannelAudioEngine::instance().assignProcessToChannel(appStd, assignedChannel);
}

void AudioRouterDock::onAppAssignedToChannel(const QString& appName, ChannelType targetChannel) {
    std::string appStd = appName.toStdString();
    uint32_t pid = 0;

    // 1. Remove de qualquer outro canal prévio e recupera o PID existente
    for (auto& [type, strip] : m_channelStrips) {
        if (strip->hasApp(appName)) {
            if (pid == 0) pid = strip->getAppPid(appName);
            if (type != targetChannel) {
                strip->removeApp(appName);
            }
        }
    }

    // 2. Adiciona na nova coluna preservando o PID
    auto it = m_channelStrips.find(targetChannel);
    if (it != m_channelStrips.end() && !it->second->hasApp(appName)) {
        it->second->addApp(appName, pid);
    }

    // 3. Salva a regra personalizada no disco e reatribui na engine de áudio
    m_rules->setCustomRule(appStd, targetChannel);
    saveRulesToDisk();
    ChannelAudioEngine::instance().assignProcessToChannel(appStd, targetChannel);
}

void AudioRouterDock::onAppMadeIgnored(const QString& appName) {
    std::string appStd = appName.toStdString();
    m_rules->setAppIgnored(appStd, true);
    saveRulesToDisk();

    // 1. Remove o badge visual de qualquer canal onde estiver exibido
    for (auto& [type, strip] : m_channelStrips) {
        if (strip->hasApp(appName)) {
            strip->removeApp(appName);
        }
    }

    // 2. Desassocia da engine de áudio para cessar qualquer atenuação/mute do plugin
    ChannelAudioEngine::instance().unassignProcess(appStd);

    // 3. Restaura o volume integral no Windows WASAPI (100% volume e desmutado)
    if (m_sessionWatcher) {
        m_sessionWatcher->setProcessVolume(appStd, 1.0f);
        m_sessionWatcher->setProcessMute(appStd, false);
    }

    updateIgnoredBtnText();
}

void AudioRouterDock::onAppIgnoredRemoved(const QString& appName) {
    std::string appStd = appName.toStdString();
    m_rules->setAppIgnored(appStd, false);
    saveRulesToDisk();
    updateIgnoredBtnText();

    // Reavalia sessões ativas do Windows para reintegrar o app caso esteja emitindo som
    refreshActiveApplications();
}

void AudioRouterDock::showIgnoredAppsDialog() {
    auto ignoredList = m_rules->getIgnoredApps();
    auto *dialog = new IgnoredAppsDialog(ignoredList, this);
    connect(dialog, &IgnoredAppsDialog::appRestored, this, [this, dialog](const QString& app) {
        onAppIgnoredRemoved(app);
        dialog->updateAppsList(m_rules->getIgnoredApps());
    });
    dialog->exec();
    dialog->deleteLater();
}

} // namespace AudioRouter
