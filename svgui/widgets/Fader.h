/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef FADER_H
#define FADER_H

/**
 * Horizontal audio fader and meter widget.
 * Based on the vertical fader and meter widget from:
 * 
 * Hydrogen
 * Copyright(c) 2002-2005 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * http://www.hydrogen-music.org
 */


#include <string>
#include <iostream>

#include <QWidget>
#include <QPixmap>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>

#include "base/Debug.h"

class Fader : public QWidget
{
    Q_OBJECT

public:
    Fader(QWidget *parent, bool withoutKnob = false);
    ~Fader();

    void setValue(float newValue);
    float getValue();

    void setPeakLeft(float);
    float getPeakLeft() { return m_peakLeft; }

    void setPeakRight(float);
    float getPeakRight() { return m_peakRight; }

signals:
    void valueChanged(float); // 0.0 -> 1.0

    void mouseEntered();
    void mouseLeft();

protected:
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseDoubleClickEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    virtual void wheelEvent( QWheelEvent *ev );
    virtual void paintEvent(QPaintEvent *ev);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

    int getMaxX() const;

    bool m_withoutKnob;
    float m_value;
    float m_peakLeft;
    float m_peakRight;

    bool m_mousePressed;
    int m_mousePressX;
    float m_mousePressValue;

    QPixmap m_back;
    QPixmap m_leds;
    QPixmap m_knob;
    QPixmap m_clip;
};

#endif
