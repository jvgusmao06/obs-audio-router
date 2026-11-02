#include "audio-router-dock.hpp"
#include "channel-strip-widget.hpp"
#include "chatmix-slider.hpp"
#include "localization-helper.hpp"
#include "core/channel-audio-engine.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QMetaObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

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
    ChannelAudioEngine::instance().setSessionWatcher(nullptr);
    if (m_sessionWatcher) {
        m_sessionWatcher->stop();
    }
}

void AudioRouterDock::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 8, 10, 10);
    rootLayout->setSpacing(8);

    // 1. Barra Superior: Logo e ChatMix Slider centralizado
    auto *topBarLayout = new QHBoxLayout();
    topBarLayout->setContentsMargins(4, 0, 4, 0);

    auto *logoLabel = new QLabel("<b>AUDIO ROUTER</b>", this);
    logoLabel->setStyleSheet("color: #ffffff; font-size: 13px; letter-spacing: 1px; font-weight: bold; border: none;");
    topBarLayout->addWidget(logoLabel);

    topBarLayout->addStretch();

    m_chatMixSlider = new ChatMixSlider(this);
    connect(m_chatMixSlider, &ChatMixSlider::chatMixChanged, [](float factor) {
        ChannelAudioEngine::instance().setChatMix(factor);
    });
    topBarLayout->addWidget(m_chatMixSlider);

    topBarLayout->addStretch();

    // Espaçador lateral para balancear o título
    auto *rightSpacer = new QWidget(this);
    rightSpacer->setFixedWidth(100);
    topBarLayout->addWidget(rightSpacer);

    rootLayout->addLayout(topBarLayout);

    // 2. Colunas de Canais Mixer (GAME, CHAT, MEDIA, SYSTEM)
    auto *channelsContainer = new QWidget(this);
    m_channelsLayout = new QHBoxLayout(channelsContainer);
    m_channelsLayout->setContentsMargins(0, 0, 0, 0);
    m_channelsLayout->setSpacing(8);

    auto addChannel = [this](ChannelType type) {
        auto *strip = new ChannelStripWidget(type, this);
        connect(strip, &ChannelStripWidget::appReassigned, this, &AudioRouterDock::onAppAssignedToChannel);
        m_channelStrips[type] = strip;
        m_channelsLayout->addWidget(strip);
    };

    addChannel(ChannelType::GAME);
    addChannel(ChannelType::CHAT);
    addChannel(ChannelType::MEDIA);
    addChannel(ChannelType::SYSTEM);

    rootLayout->addWidget(channelsContainer);
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
    for (const auto& s : sessions) {
        if (!s.processName.empty()) {
            onNewSessionDetected(QString::fromStdString(s.processName), s.processId);
        }
    }
}

void AudioRouterDock::onNewSessionDetected(const QString& appName, uint32_t pid) {
    std::string appStd = appName.toStdString();

    // 1. Resolve para qual canal o aplicativo deve ir (regra customizada ou padrão)
    ChannelType assignedChannel = m_rules->resolveChannel(appStd);

    // 2. Insere na seção de Apps da coluna correspondente (se ainda não adicionado visualmente)
    auto it = m_channelStrips.find(assignedChannel);
    if (it != m_channelStrips.end() && !it->second->hasApp(appName)) {
        it->second->addApp(appName, pid);
    }

    // 3. SEMPRE registra na engine e aplica volume/mute na nova sessão WASAPI
    // Previne que apps reabertos toquem no volume padrão de 100%
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

} // namespace AudioRouter
