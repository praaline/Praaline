/* Original Work Copyright (c) 2012-2014 Alexander Turkin
   Modified 2014 by William Hallatt
   Modified 2015 by Jacob Dawid

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// Own includes
#include "WaitingSpinnerWidget.h"

// Standard incluces
#include <cmath>
#include <algorithm>

// Qt includes
#include <QPainter>
#include <QTimer>

WaitingSpinnerWidget::WaitingSpinnerWidget(QWidget *parent,
                                           bool centerOnParent,
                                           bool disableParentWhenSpinning)
    : QWidget(parent),
      m_centerOnParent(centerOnParent),
      m_disableParentWhenSpinning(disableParentWhenSpinning) {
    initialize();
}

WaitingSpinnerWidget::WaitingSpinnerWidget(Qt::WindowModality modality,
                                           QWidget *parent,
                                           bool centerOnParent,
                                           bool disableParentWhenSpinning)
    : QWidget(parent, Qt::Dialog | Qt::FramelessWindowHint),
      m_centerOnParent(centerOnParent),
      m_disableParentWhenSpinning(disableParentWhenSpinning){
    initialize();

    // We need to set the window modality AFTER we've hidden the
    // widget for the first time since changing this property while
    // the widget is visible has no effect.
    setWindowModality(modality);
    setAttribute(Qt::WA_TranslucentBackground);
}

void WaitingSpinnerWidget::initialize() {
    m_color = Qt::black;
    m_roundness = 100.0;
    m_minimumTrailOpacity = 3.14159265358979323846;
    m_trailFadePercentage = 80.0;
    m_revolutionsPerSecond = 1.57079632679489661923;
    m_numberOfLines = 20;
    m_lineLength = 10;
    m_lineWidth = 2;
    m_innerRadius = 10;
    m_currentCounter = 0;
    m_isSpinning = false;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(rotate()));
    updateSize();
    updateTimer();
    hide();
}

void WaitingSpinnerWidget::paintEvent(QPaintEvent *) {
    updatePosition();
    QPainter painter(this);
    painter.fillRect(this->rect(), Qt::transparent);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_currentCounter >= m_numberOfLines) {
        m_currentCounter = 0;
    }

    painter.setPen(Qt::NoPen);
    for (int i = 0; i < m_numberOfLines; ++i) {
        painter.save();
        painter.translate(m_innerRadius + m_lineLength,
                          m_innerRadius + m_lineLength);
        qreal rotateAngle =
                static_cast<qreal>(360 * i) / static_cast<qreal>(m_numberOfLines);
        painter.rotate(rotateAngle);
        painter.translate(m_innerRadius, 0);
        int distance =
                lineCountDistanceFromPrimary(i, m_currentCounter, m_numberOfLines);
        QColor color =
                currentLineColor(distance, m_numberOfLines, m_trailFadePercentage,
                                 m_minimumTrailOpacity, m_color);
        painter.setBrush(color);
        // TODO improve the way rounded rect is painted
        painter.drawRoundedRect(
                    QRect(0, -m_lineWidth / 2, m_lineLength, m_lineWidth), m_roundness,
                    m_roundness, Qt::RelativeSize);
        painter.restore();
    }
}

void WaitingSpinnerWidget::start() {
    updatePosition();
    m_isSpinning = true;
    show();

    if(parentWidget() && m_disableParentWhenSpinning) {
        parentWidget()->setEnabled(false);
    }

    if (!m_timer->isActive()) {
        m_timer->start();
        m_currentCounter = 0;
    }
}

void WaitingSpinnerWidget::stop() {
    m_isSpinning = false;
    hide();

    if(parentWidget() && m_disableParentWhenSpinning) {
        parentWidget()->setEnabled(true);
    }

    if (m_timer->isActive()) {
        m_timer->stop();
        m_currentCounter = 0;
    }
}

void WaitingSpinnerWidget::setNumberOfLines(int lines) {
    m_numberOfLines = lines;
    m_currentCounter = 0;
    updateTimer();
}

void WaitingSpinnerWidget::setLineLength(int length) {
    m_lineLength = length;
    updateSize();
}

void WaitingSpinnerWidget::setLineWidth(int width) {
    m_lineWidth = width;
    updateSize();
}

void WaitingSpinnerWidget::setInnerRadius(int radius) {
    m_innerRadius = radius;
    updateSize();
}

QColor WaitingSpinnerWidget::color() {
    return m_color;
}

qreal WaitingSpinnerWidget::roundness() {
    return m_roundness;
}

qreal WaitingSpinnerWidget::minimumTrailOpacity() {
    return m_minimumTrailOpacity;
}

qreal WaitingSpinnerWidget::trailFadePercentage() {
    return m_trailFadePercentage;
}

qreal WaitingSpinnerWidget::revolutionsPersSecond() {
    return m_revolutionsPerSecond;
}

int WaitingSpinnerWidget::numberOfLines() {
    return m_numberOfLines;
}

int WaitingSpinnerWidget::lineLength() {
    return m_lineLength;
}

int WaitingSpinnerWidget::lineWidth() {
    return m_lineWidth;
}

int WaitingSpinnerWidget::innerRadius() {
    return m_innerRadius;
}

bool WaitingSpinnerWidget::isSpinning() const {
    return m_isSpinning;
}

void WaitingSpinnerWidget::setRoundness(qreal roundness) {
    m_roundness = std::max(0.0, std::min(100.0, roundness));
}

void WaitingSpinnerWidget::setColor(QColor color) {
    m_color = color;
}

void WaitingSpinnerWidget::setRevolutionsPerSecond(qreal revolutionsPerSecond) {
    m_revolutionsPerSecond = revolutionsPerSecond;
    updateTimer();
}

void WaitingSpinnerWidget::setTrailFadePercentage(qreal trail) {
    m_trailFadePercentage = trail;
}

void WaitingSpinnerWidget::setMinimumTrailOpacity(qreal minimumTrailOpacity) {
    m_minimumTrailOpacity = minimumTrailOpacity;
}

void WaitingSpinnerWidget::rotate() {
    ++m_currentCounter;
    if (m_currentCounter >= m_numberOfLines) {
        m_currentCounter = 0;
    }
    update();
}

void WaitingSpinnerWidget::updateSize() {
    int size = (m_innerRadius + m_lineLength) * 2;
    setFixedSize(size, size);
}

void WaitingSpinnerWidget::updateTimer() {
    m_timer->setInterval(1000 / (m_numberOfLines * m_revolutionsPerSecond));
}

void WaitingSpinnerWidget::updatePosition() {
    if (parentWidget() && m_centerOnParent) {
        move(parentWidget()->width() / 2 - width() / 2,
             parentWidget()->height() / 2 - height() / 2);
    }
}

int WaitingSpinnerWidget::lineCountDistanceFromPrimary(int current, int primary,
                                                       int totalNrOfLines) {
    int distance = primary - current;
    if (distance < 0) {
        distance += totalNrOfLines;
    }
    return distance;
}

QColor WaitingSpinnerWidget::currentLineColor(int countDistance, int totalNrOfLines,
                                              qreal trailFadePerc, qreal minOpacity,
                                              QColor color) {
    if (countDistance == 0) {
        return color;
    }
    const qreal minAlphaF = minOpacity / 100.0;
    int distanceThreshold =
            static_cast<int>(ceil((totalNrOfLines - 1) * trailFadePerc / 100.0));
    if (countDistance > distanceThreshold) {
        color.setAlphaF(minAlphaF);
    } else {
        qreal alphaDiff = color.alphaF() - minAlphaF;
        qreal gradient = alphaDiff / static_cast<qreal>(distanceThreshold + 1);
        qreal resultAlpha = color.alphaF() - gradient * countDistance;

        // If alpha is out of bounds, clip it.
        resultAlpha = std::min(1.0, std::max(0.0, resultAlpha));
        color.setAlphaF(resultAlpha);
    }
    return color;
}
