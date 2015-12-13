/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Panner.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QPainter>

#include <iostream>
#include <cmath>

Panner::Panner(QWidget *parent) :
    QWidget(parent),
    m_rectX(0),
    m_rectY(0),
    m_rectWidth(1),
    m_rectHeight(1),
    m_scrollUnit(0),
    m_defaultCentreX(0),
    m_defaultCentreY(0),
    m_defaultsSet(false),
    m_thumbColour(palette().highlightedText().color()),
    m_backgroundAlpha(255),
    m_thumbAlpha(255),
    m_clicked(false),
    m_dragStartX(0),
    m_dragStartY(0)
{
}

Panner::~Panner()
{
}

void
Panner::setAlpha(int backgroundAlpha, int thumbAlpha)
{
    m_backgroundAlpha = backgroundAlpha;
    m_thumbAlpha = thumbAlpha;
}

void
Panner::setScrollUnit(float unit)
{
    m_scrollUnit = unit;
}

void
Panner::scroll(bool up)
{
    float unit = m_scrollUnit;
    if (unit == 0.f) {
        unit = float(m_rectHeight) / (6 * float(height()));
        if (unit < 0.01f) unit = 0.01f;
    }

    if (!up) {
        m_rectY += unit;
    } else {
        m_rectY -= unit;
    }

    normalise();
    emitAndUpdate();
}

void
Panner::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MidButton ||
        ((e->button() == Qt::LeftButton) &&
         (e->modifiers() & Qt::ControlModifier))) {
        resetToDefault();
    } else if (e->button() == Qt::LeftButton) {
        m_clicked = true;
        m_clickPos = e->pos();
        m_dragStartX = m_rectX;
        m_dragStartY = m_rectY;
    }
}

void
Panner::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        return;
    }

    emit doubleClicked();
}

void
Panner::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_clicked) return;

    float dx = float(e->pos().x() - m_clickPos.x()) / float(width());
    float dy = float(e->pos().y() - m_clickPos.y()) / float(height());
    
    m_rectX = m_dragStartX + dx;
    m_rectY = m_dragStartY + dy;
    
    normalise();
    repaint();
    emit rectExtentsChanged(m_rectX, m_rectY, m_rectWidth, m_rectHeight);
    emit rectCentreMoved(centreX(), centreY());
}

void
Panner::mouseReleaseEvent(QMouseEvent *e)
{
    if (!m_clicked) return;

    mouseMoveEvent(e);
    m_clicked = false;
}

void
Panner::wheelEvent(QWheelEvent *e)
{
    scroll(e->delta() > 0);
}

void
Panner::enterEvent(QEvent *)
{
    emit mouseEntered();
}

void
Panner::leaveEvent(QEvent *)
{
    emit mouseLeft();
}

void
Panner::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, false);

    QColor bg(palette().background().color());
    bg.setAlpha(m_backgroundAlpha);

    paint.setPen(palette().dark().color());
    paint.setBrush(bg);
    paint.drawRect(0, 0, width()-1, height()-1);

    QColor hl(m_thumbColour);
    hl.setAlpha(m_thumbAlpha);

    paint.setBrush(hl);

    int rw = int(lrintf(float(width() - 1) * m_rectWidth));
    int rh = int(lrintf(float(height() - 1) * m_rectHeight));
    if (rw < 2) rw = 2;
    if (rh < 2) rh = 2;

    paint.drawRect(int(lrintf(float(width()) * m_rectX)),
                   int(lrintf(float(height()) * m_rectY)),
                   rw, rh);
}

void
Panner::normalise()
{
    if (m_rectWidth > 1.f) m_rectWidth = 1.f;
    if (m_rectHeight > 1.f) m_rectHeight = 1.f;
    if (m_rectX + m_rectWidth > 1.f) m_rectX = 1.f - m_rectWidth;
    if (m_rectX < 0) m_rectX = 0;
    if (m_rectY + m_rectHeight > 1.f) m_rectY = 1.f - m_rectHeight;
    if (m_rectY < 0) m_rectY = 0;

    if (!m_defaultsSet) {
        m_defaultCentreX = centreX();
        m_defaultCentreY = centreY();
        m_defaultsSet = true;
    }
}

void
Panner::emitAndUpdate()
{
    emit rectExtentsChanged(m_rectX, m_rectY, m_rectWidth, m_rectHeight);
    emit rectCentreMoved(centreX(), centreY());
    update();
}  

void
Panner::getRectExtents(float &x0, float &y0, float &width, float &height)
{
    x0 = m_rectX;
    y0 = m_rectY;
    width = m_rectWidth;
    height = m_rectHeight;
}

void
Panner::setRectExtents(float x0, float y0, float width, float height)
{
//    cerr << "Panner::setRectExtents(" << x0 << ", " << y0 << ", "
//              << width << ", " << height << ")" << endl;

    if (m_rectX == x0 &&
        m_rectY == y0 &&
        m_rectWidth == width &&
        m_rectHeight == height) {
        return;
    }

    m_rectX = x0;
    m_rectY = y0;
    m_rectWidth = width;
    m_rectHeight = height;

    normalise();
    emitAndUpdate();
}

void
Panner::setRectWidth(float width)
{
    if (m_rectWidth == width) return;
    m_rectWidth = width;
    normalise();
    emitAndUpdate();
}

void
Panner::setRectHeight(float height)
{
    if (m_rectHeight == height) return;
    m_rectHeight = height;
    normalise();
    emitAndUpdate();
}

void
Panner::setRectCentreX(float x)
{
    float x0 = x - m_rectWidth/2;
    if (x0 == m_rectX) return;
    m_rectX = x0;
    normalise();
    emitAndUpdate();
}

void
Panner::setRectCentreY(float y)
{
    float y0 = y - m_rectHeight/2;
    if (y0 == m_rectY) return;
    m_rectY = y0;
    normalise();
    emitAndUpdate();
}

QSize
Panner::sizeHint() const
{
    return QSize(30, 30);
}

void
Panner::setDefaultRectCentre(float cx, float cy)
{
    m_defaultCentreX = cx;
    m_defaultCentreY = cy;
    m_defaultsSet = true;
}

void
Panner::resetToDefault()
{
    float x0 = m_defaultCentreX - m_rectWidth/2;
    float y0 = m_defaultCentreY - m_rectHeight/2;
    if (x0 == m_rectX && y0 == m_rectY) return;
    m_rectX = x0;
    m_rectY = y0;
    normalise();
    emitAndUpdate();
}


