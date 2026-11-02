#include "chatmix-slider.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QEvent>
#include <algorithm>

namespace AudioRouter {

ChatMixSlider::ChatMixSlider(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void ChatMixSlider::setupUi() {
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(6, 4, 6, 4);
    rootLayout->setSpacing(2);

    m_titleLabel = new QLabel("CHATMIX", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("color: #8e9297; font-size: 10px; font-weight: bold; letter-spacing: 1px;");
    rootLayout->addWidget(m_titleLabel);

    auto *rowLayout = new QHBoxLayout();
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(8);

    m_gameIcon = new QLabel("🎮", this);
    m_gameIcon->setStyleSheet("font-size: 13px; color: #00f0aa;");
    m_gameIcon->setToolTip("100% Game Audio");
    rowLayout->addWidget(m_gameIcon);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(-100, 100);
    m_slider->setValue(0);
    m_slider->setFixedWidth(160);
    m_slider->setToolTip("ChatMix: Balanço entre Jogo (esquerda) e Chat (direita). Clique duplo para centralizar.");
    m_slider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    background: #18191c;"
        "    height: 6px;"
        "    border-radius: 3px;"
        "    border: 1px solid #202225;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: #dcddde;"
        "    width: 12px;"
        "    height: 18px;"
        "    margin: -6px 0;"
        "    border-radius: 3px;"
        "    border: 1px solid #72767d;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "    background: #ffffff;"
        "    border-color: #00f0aa;"
        "}"
    );

    // Filtro de evento para resetar no duplo-clique
    m_slider->installEventFilter(this);

    connect(m_slider, &QSlider::valueChanged, this, [this](int val) {
        emit chatMixChanged(static_cast<float>(val) / 100.0f);
    });

    rowLayout->addWidget(m_slider);

    m_chatIcon = new QLabel("💬", this);
    m_chatIcon->setStyleSheet("font-size: 13px; color: #5865f2;");
    m_chatIcon->setToolTip("100% Chat Voice Audio");
    rowLayout->addWidget(m_chatIcon);

    rootLayout->addLayout(rowLayout);
}

float ChatMixSlider::chatMix() const {
    return static_cast<float>(m_slider->value()) / 100.0f;
}

void ChatMixSlider::setChatMix(float factor) {
    factor = std::clamp(factor, -1.0f, 1.0f);
    m_slider->blockSignals(true);
    m_slider->setValue(static_cast<int>(factor * 100.0f));
    m_slider->blockSignals(false);
}

bool ChatMixSlider::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_slider && event->type() == QEvent::MouseButtonDblClick) {
        m_slider->setValue(0); // Reseta ao centro
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

} // namespace AudioRouter
