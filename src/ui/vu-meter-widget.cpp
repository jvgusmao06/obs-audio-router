/**
 * @file vu-meter-widget.cpp
 * @brief Implementation of VuMeterWidget drawing responsive peak levels.
 */

#include "vu-meter-widget.hpp"
#include <QPainter>
#include <QLinearGradient>
#include <algorithm>

namespace AudioRouter {

VuMeterWidget::VuMeterWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(4, 105);
}

void VuMeterWidget::setLevel(float level) {
    level = std::clamp(level, 0.0f, 1.0f);
    // Quick rise, smooth decay
    if (level >= m_level) {
        m_level = level;
    } else {
        m_level = m_level * 0.82f;
        if (m_level < 0.005f) m_level = 0.0f;
    }
    update();
}

void VuMeterWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // Background trough
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#18191c"));
    painter.drawRoundedRect(0, 0, w, h, 2, 2);

    if (m_level > 0.001f) {
        int barHeight = static_cast<int>(h * m_level);
        if (barHeight < 2) barHeight = 2;
        int barY = h - barHeight;

        // Vertical gradient: Green (bottom) -> Amber (middle) -> Red (top)
        QLinearGradient grad(0, h, 0, 0);
        grad.setColorAt(0.0f, QColor("#10b981"));
        grad.setColorAt(0.7f, QColor("#f59e0b"));
        grad.setColorAt(1.0f, QColor("#ef4444"));

        painter.setBrush(grad);
        painter.drawRoundedRect(0, barY, w, barHeight, 2, 2);
    }
}

} // namespace AudioRouter
