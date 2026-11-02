#pragma once

#include <QWidget>

class QSlider;
class QLabel;

namespace AudioRouter {

class ChatMixSlider : public QWidget {
    Q_OBJECT

public:
    explicit ChatMixSlider(QWidget *parent = nullptr);
    ~ChatMixSlider() override = default;

    float chatMix() const;
    void setChatMix(float factor);

signals:
    void chatMixChanged(float factor);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUi();

    QSlider *m_slider = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_gameIcon = nullptr;
    QLabel *m_chatIcon = nullptr;
};

} // namespace AudioRouter

