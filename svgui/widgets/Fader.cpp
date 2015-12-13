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

/**
 * Horizontal audio fader and meter widget.
 *
 * Based on the vertical fader and meter widget from the Hydrogen drum
 * machine.  (Any poor taste that has crept in during the
 * modifications for this application is entirely my own, however.)
 * The following copyright notice applies to code from this file, and
 * also to the files in icons/fader_*.png (also modified by me). --cc
 */

/**
 * Hydrogen
 * Copyright(c) 2002-2005 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 * http://www.hydrogen-music.org
 */


#include "Fader.h"

#include "base/AudioLevel.h"

#include <QMouseEvent>
#include <QPixmap>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QInputDialog>

#include <iostream>

Fader::Fader(QWidget *parent, bool withoutKnob) :
    QWidget(parent),
    m_withoutKnob(withoutKnob),
    m_value(1.0),
    m_peakLeft(0.0),
    m_peakRight(0.0),
    m_mousePressed(false),
    m_mousePressX(0),
    m_mousePressValue(0)
{
    setMinimumSize(116, 23);
    setMaximumSize(116, 23);
    resize(116, 23);

    QString background_path = ":/icons/visualiser/fader_background.png";
    bool ok = m_back.load(background_path);
    if (ok == false) {
	cerr << "Fader: Error loading pixmap" << endl;
    }

    QString leds_path = ":/icons/visualiser/fader_leds.png";
    ok = m_leds.load(leds_path);
    if (ok == false) {
	cerr <<  "Error loading pixmap" << endl;
    }

    QString knob_path = ":/icons/visualiser/fader_knob.png";
    ok = m_knob.load(knob_path);
    if (ok == false) {
	cerr <<  "Error loading pixmap" << endl;
    }

    QString clip_path = ":/icons/visualiser/fader_knob_red.png";
    ok = m_clip.load(clip_path);
    if (ok == false) {
	cerr <<  "Error loading pixmap" << endl;
    }
}

Fader::~Fader()
{

}

void
Fader::mouseMoveEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::MidButton) {
        setValue(1.0);
        emit valueChanged(1.0);
        update();
        ev->accept();
        return;
    }
    if (!m_mousePressed) return;

    int x = ev->x();
    int diff = x - m_mousePressX;
    if (diff == 0) return;

    int vx = AudioLevel::multiplier_to_fader
        (m_mousePressValue, getMaxX(), AudioLevel::LongFader);

    vx += diff;

    if (vx > getMaxX()) vx = getMaxX();
    if (vx < 0) vx = 0;

    float fval = (float)AudioLevel::fader_to_multiplier
	(vx, getMaxX(), AudioLevel::LongFader);

    setValue(fval);
    emit valueChanged(fval);
    ev->accept();
}


void
Fader::mouseReleaseEvent(QMouseEvent *ev)
{
    if (m_mousePressed) {
        mouseMoveEvent(ev);
        m_mousePressed = false;
    }
}

void
Fader::mouseDoubleClickEvent(QMouseEvent *)
{
    bool ok = false;
    float min = (float)AudioLevel::fader_to_dB
        (0, getMaxX(), AudioLevel::LongFader);
    float max = (float)AudioLevel::fader_to_dB
        (getMaxX(), getMaxX(), AudioLevel::LongFader);
    float deft = (float)AudioLevel::multiplier_to_dB(m_value);

    float dB = (float)QInputDialog::getDouble
        (this,
         tr("Enter new fader level"),
         tr("New fader level, from %1 to %2 dBFS:").arg(min).arg(max),
         deft, min, max, 3, &ok);

    if (ok) {
        float value = (float)AudioLevel::dB_to_multiplier(dB);
        setValue(value);
        emit valueChanged(value);
        update();
    }
}

void
Fader::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::MidButton ||
        ((ev->button() == Qt::LeftButton) &&
         (ev->modifiers() & Qt::ControlModifier))) {
        setValue(1.0);
        emit valueChanged(1.0);
        update();
        return;
    }

    if (ev->button() != Qt::LeftButton) return;
    m_mousePressed = true;
    m_mousePressX = ev->x();
    m_mousePressValue = getValue();
}


void
Fader::wheelEvent(QWheelEvent *ev)
{
    ev->accept();

    //!!! needs improvement

    if (ev->delta() > 0) {
	setValue(m_value * 1.f);
    } else {
	setValue(m_value / 1.f);
    }

    update();
    emit valueChanged(getValue());
}

void
Fader::enterEvent(QEvent *)
{
    emit mouseEntered();
}

void
Fader::leaveEvent(QEvent *)
{
    emit mouseLeft();
}

void
Fader::setValue(float v)
{
    float max = (float)AudioLevel::dB_to_multiplier(10.0);

    if (v > max) {
	v = max;
    } else if (v < 0.0) {
	v = 0.0;
    }

    if (m_value != v) {
	m_value = v;
	float db = (float)AudioLevel::multiplier_to_dB(m_value);
        QString text;
	if (db <= AudioLevel::DB_FLOOR) {
            text = tr("Level: Off");
	} else {
            text = tr("Level: %1%2.%3%4 dB")
                .arg(db < 0.0 ? "-" : "")
                .arg(abs(int(db)))
                .arg(abs(int(db * 10.0) % 10))
                .arg(abs(int(db * 100.0) % 10));
	}
        cerr << "Fader: setting tooltip to \"" << text << "\"" << endl;
        QWidget::setToolTip(text);
	update();
    }
}


float
Fader::getValue()
{
    return m_value;
}



void
Fader::setPeakLeft(float peak)
{
    if (this->m_peakLeft != peak) {
	this->m_peakLeft = peak;
	update();
    }
}


void
Fader::setPeakRight(float peak) 
{
    if (this->m_peakRight != peak) {
	this->m_peakRight = peak;
	update();
    }
}


void
Fader::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // background
    painter.drawPixmap(rect(), m_back, QRect(0, 0, 116, 23));

    int offset_L = AudioLevel::multiplier_to_fader(m_peakLeft, 116,
						   AudioLevel::IEC268LongMeter);

    painter.drawPixmap(QRect(0, 0, offset_L, 11), m_leds,
		       QRect(0, 0, offset_L, 11));

    int offset_R = AudioLevel::multiplier_to_fader(m_peakRight, 116,
						   AudioLevel::IEC268LongMeter);

    painter.drawPixmap(QRect(0, 11, offset_R, 11), m_leds,
		       QRect(0, 11, offset_R, 11));

    if (m_withoutKnob == false) {

	static const uint knob_width = 29;
	static const uint knob_height = 9;

	int x = AudioLevel::multiplier_to_fader(m_value, 116 - knob_width,
						AudioLevel::LongFader);

	bool clipping = (m_peakLeft > 1.0 || m_peakRight > 1.0);

	painter.drawPixmap(QRect(x, 7, knob_width, knob_height),
			   clipping ? m_clip : m_knob,
			   QRect(0, 0, knob_width, knob_height));
    }
}

int
Fader::getMaxX() const
{
    return 116 - 12;
}
