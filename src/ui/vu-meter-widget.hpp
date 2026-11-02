/**
 * @file vu-meter-widget.hpp
 * @brief Compact vertical real-time audio peak VU meter widget with smooth decay and gradient.
 */

#pragma once

#include <QWidget>

namespace AudioRouter {

class VuMeterWidget : public QWidget {
    Q_OBJECT

public:
    explicit VuMeterWidget(QWidget *parent = nullptr);
    ~VuMeterWidget() override = default;

    void setLevel(float level);
    float level() const { return m_level; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    float m_level = 0.0f;
};

} // namespace AudioRouter

