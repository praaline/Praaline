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

/*
    This is a modified version of a source file from the KDE
    libraries.  Copyright (c) 1998-2004 Jörg Habenicht, Richard J
    Moore and others, distributed under the GNU Lesser General Public
    License.

    Ported to Qt4 by Chris Cannam.

    The original KDE widget comes in round and rectangular and flat,
    raised, and sunken variants.  This version retains only the round
    sunken variant.  This version also implements a simple button API.
*/

#ifndef _LED_BUTTON_H_
#define _LED_BUTTON_H_

#include <QWidget>
#include "base/Debug.h"

class QColor;

class LEDButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(int darkFactor READ darkFactor WRITE setDarkFactor)

public:
    LEDButton(QWidget *parent = 0);
    LEDButton(const QColor &col, QWidget *parent = 0);
    LEDButton(const QColor& col, bool state, QWidget *parent = 0);
    ~LEDButton();

    bool state() const;
    QColor color() const;
    int darkFactor() const;

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

signals:
    void stateChanged(bool);

    void mouseEntered();
    void mouseLeft();

public slots:
    void toggle();
    void on();
    void off();

    void setState(bool);
    void toggleState();
    void setColor(const QColor& color);
    void setDarkFactor(int darkfactor);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    bool led_state;
    QColor led_color;

    class LEDButtonPrivate;
    LEDButtonPrivate *d;
};

#endif
