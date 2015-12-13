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
 * A rotary dial widget.
 *
 * Based on an original design by Thorsten Wilms.
 *
 * Implemented as a widget for the Rosegarden MIDI and audio sequencer
 * and notation editor by Chris Cannam.
 *
 * Extracted into a standalone Qt3 widget by Pedro Lopez-Cabanillas
 * and adapted for use in QSynth.
 * 
 * Ported to Qt4 by Chris Cannam.
 *
 * This file copyright 2003-2006 Chris Cannam, copyright 2005 Pedro
 * Lopez-Cabanillas, copyright 2006 Queen Mary, University of London.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.  See the file
 * COPYING included with this distribution for more information.
 */

#include "AudioDial.h"

#include "base/RangeMapper.h"

#include <cmath>
#include <iostream>

#include <QTimer>
#include <QPainter>
#include <QPixmap>
#include <QColormap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QInputDialog>

#include "base/Profiler.h"





//!!! Pedro updated his version to use my up/down response code from RG -- need to grab that code in preference to this version from Rui


//-------------------------------------------------------------------------
// AudioDial - Instance knob widget class.
//

#define AUDIO_DIAL_MIN (0.25 * M_PI)
#define AUDIO_DIAL_MAX (1.75 * M_PI)
#define AUDIO_DIAL_RANGE (AUDIO_DIAL_MAX - AUDIO_DIAL_MIN)


//static int dialsExtant = 0;


// Constructor.
AudioDial::AudioDial(QWidget *parent) :
    QDial(parent),
    m_knobColor(Qt::black),
    m_meterColor(Qt::white),
    m_defaultValue(0),
    m_defaultMappedValue(0),
    m_mappedValue(0),
    m_noMappedUpdate(false),
    m_showTooltip(true),
    m_rangeMapper(0)
{
    m_mouseDial = false;
    m_mousePressed = false;
//    ++dialsExtant;
}


// Destructor.
AudioDial::~AudioDial (void)
{
    delete m_rangeMapper;
//    --dialsExtant;
}


void AudioDial::setRangeMapper(RangeMapper *mapper)
{
//    cerr << "AudioDial[" << this << "][\"" << objectName() << "\"::setRangeMapper(" << mapper << ") [current is " << m_rangeMapper << "] (have " << dialsExtant << " dials extant)" << endl;

    if (m_rangeMapper == mapper) return;

    if (!m_rangeMapper && mapper) {
        connect(this, SIGNAL(valueChanged(int)),
                this, SLOT(updateMappedValue(int)));
    }

    delete m_rangeMapper;
    m_rangeMapper = mapper;

    updateMappedValue(value());
}


void AudioDial::paintEvent(QPaintEvent *)
{
    Profiler profiler("AudioDial::paintEvent");

    QPainter paint;

    double angle = AUDIO_DIAL_MIN // offset
	+ (AUDIO_DIAL_RANGE *
	   (double(QDial::value() - QDial::minimum()) /
	    (double(QDial::maximum() - QDial::minimum()))));
    int degrees = int(angle * 180.0 / M_PI);

    int ns = notchSize();
    int numTicks = 1 + (maximum() + ns - minimum()) / ns;
	
    QColor knobColor(m_knobColor);
    if (knobColor == Qt::black)
	knobColor = palette().window().color();

    QColor meterColor(m_meterColor);
    if (!isEnabled())
	meterColor = palette().mid().color();
    else if (m_meterColor == Qt::white)
	meterColor = palette().highlight().color();

    int m_size = width() < height() ? width() : height();
    int scale = 1;
    int width = m_size - 2*scale;

    paint.begin(this);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.translate(1, 1);

    QPen pen;
    QColor c;

    // Knob body and face...

    c = knobColor;
    pen.setColor(knobColor);
    pen.setWidth(scale * 2);
    pen.setCapStyle(Qt::FlatCap);
	
    paint.setPen(pen);
    paint.setBrush(c);

    int indent = (int)(width * 0.15 + 1);

    paint.drawEllipse(indent-1, indent-1, width-2*indent, width-2*indent);

    pen.setWidth(3 * scale);
    int pos = indent-1 + (width-2*indent) / 20;
    int darkWidth = (width-2*indent) * 3 / 4;
    while (darkWidth) {
	c = c.light(102);
	pen.setColor(c);
	paint.setPen(pen);
	paint.drawEllipse(pos, pos, darkWidth, darkWidth);
	if (!--darkWidth) break;
	paint.drawEllipse(pos, pos, darkWidth, darkWidth);
	if (!--darkWidth) break;
	paint.drawEllipse(pos, pos, darkWidth, darkWidth);
	++pos; --darkWidth;
    }

    // Tick notches...

    if ( notchesVisible() ) {
//	cerr << "Notches visible" << endl;
	pen.setColor(palette().dark().color());
	pen.setWidth(scale);
	paint.setPen(pen);
	for (int i = 0; i < numTicks; ++i) {
	    int div = numTicks;
	    if (div > 1) --div;
	    drawTick(paint, AUDIO_DIAL_MIN + (AUDIO_DIAL_MAX - AUDIO_DIAL_MIN) * i / div,
		     width, true);
	}
    }

    // The bright metering bit...

    c = meterColor;
    pen.setColor(c);
    pen.setWidth(indent);
    paint.setPen(pen);

//    cerr << "degrees " << degrees << ", gives us " << -(degrees - 45) * 16 << endl;

    int arcLen = -(degrees - 45) * 16;
    if (arcLen == 0) arcLen = -16;

    paint.drawArc(indent/2, indent/2,
		  width-indent, width-indent, (180 + 45) * 16, arcLen);

    paint.setBrush(Qt::NoBrush);

    // Shadowing...

    pen.setWidth(scale);
    paint.setPen(pen);

    // Knob shadow...

    int shadowAngle = -720;
    c = knobColor.dark();
    for (int arc = 120; arc < 2880; arc += 240) {
	pen.setColor(c);
	paint.setPen(pen);
	paint.drawArc(indent, indent,
		      width-2*indent, width-2*indent, shadowAngle + arc, 240);
	paint.drawArc(indent, indent,
		      width-2*indent, width-2*indent, shadowAngle - arc, 240);
	c = c.light(110);
    }

    // Scale shadow, omitting the bottom part...

    shadowAngle = 2160;
    c = palette().shadow().color();
    for (int i = 0; i < 5; ++i) {
	pen.setColor(c);
	paint.setPen(pen);
        int arc = i * 240 + 120;
	paint.drawArc(scale/2, scale/2,
		      width-scale, width-scale, shadowAngle + arc, 240);
	c = c.light(110);
    }
    c = palette().shadow().color();
    for (int i = 0; i < 12; ++i) {
	pen.setColor(c);
	paint.setPen(pen);
        int arc = i * 240 + 120;
	paint.drawArc(scale/2, scale/2,
		      width-scale, width-scale, shadowAngle - arc, 240);
	c = c.light(110);
    }

    // Scale ends...

    pen.setColor(palette().shadow().color());
    pen.setWidth(scale);
    paint.setPen(pen);
    for (int i = 0; i < numTicks; ++i) {
	if (i != 0 && i != numTicks - 1) continue;
	int div = numTicks;
	if (div > 1) --div;
	drawTick(paint, AUDIO_DIAL_MIN + (AUDIO_DIAL_MAX - AUDIO_DIAL_MIN) * i / div,
		 width, false);
    }

    // Pointer notch...

    double hyp = double(width) / 2.0;
    double len = hyp - indent;
    --len;

    double x0 = hyp;
    double y0 = hyp;

    double x = hyp - len * sin(angle);
    double y = hyp + len * cos(angle);

    c = palette().dark().color();
    pen.setColor(isEnabled() ? c.dark(130) : c);
    pen.setWidth(scale * 2);
    paint.setPen(pen);
    paint.drawLine(int(x0), int(y0), int(x), int(y));

    paint.end();
}


void AudioDial::drawTick(QPainter &paint,
			 double angle, int size, bool internal)
{
    double hyp = double(size) / 2.0;
    double x0 = hyp - (hyp - 1) * sin(angle);
    double y0 = hyp + (hyp - 1) * cos(angle);

//    cerr << "drawTick: angle " << angle << ", size " << size << ", internal " << internal << endl;
    
    if (internal) {

	double len = hyp / 4;
	double x1 = hyp - (hyp - len) * sin(angle);
	double y1 = hyp + (hyp - len) * cos(angle);
		
	paint.drawLine(int(x0), int(y0), int(x1), int(y1));

    } else {

	double len = hyp / 4;
	double x1 = hyp - (hyp + len) * sin(angle);
	double y1 = hyp + (hyp + len) * cos(angle);

	paint.drawLine(int(x0), int(y0), int(x1), int(y1));
    }
}


void AudioDial::setKnobColor(const QColor& color)
{
    m_knobColor = color;
    update();
}


void AudioDial::setMeterColor(const QColor& color)
{
    m_meterColor = color;
    update();
}


void AudioDial::setMouseDial(bool mouseDial)
{
    m_mouseDial = mouseDial;
}


void AudioDial::setDefaultValue(int defaultValue)
{
    m_defaultValue = defaultValue;
    if (m_rangeMapper) {
        m_defaultMappedValue = m_rangeMapper->getValueForPosition(defaultValue);
    }
}

void AudioDial::setValue(int value)
{
    QDial::setValue(value);
    updateMappedValue(value);
}

void AudioDial::setDefaultMappedValue(double value)
{
    m_defaultMappedValue = value;
    if (m_rangeMapper) {
        m_defaultValue = m_rangeMapper->getPositionForValue(value);
    }
}

void AudioDial::setMappedValue(double mappedValue)
{
    if (m_rangeMapper) {
        int newPosition = m_rangeMapper->getPositionForValue(mappedValue);
        bool changed = (m_mappedValue != mappedValue);
        m_mappedValue = mappedValue;
        m_noMappedUpdate = true;
        cerr << "AudioDial::setMappedValue(" << mappedValue << "): new position is " << newPosition << endl;
        if (newPosition != value()) {
            setValue(newPosition);
        } else if (changed) {
            emit valueChanged(newPosition);
        }
        m_noMappedUpdate = false;
    } else {
        setValue(int(mappedValue));
    }
}


void AudioDial::setShowToolTip(bool show)
{
    m_showTooltip = show;
    m_noMappedUpdate = true;
    updateMappedValue(value());
    m_noMappedUpdate = false;
}


double AudioDial::mappedValue() const
{
    if (m_rangeMapper) {
//        cerr << "AudioDial::mappedValue(): value = " << value() << ", mappedValue = " << m_mappedValue << endl;
        return m_mappedValue;
    }
    return value();
}


void AudioDial::updateMappedValue(int value)
{
    if (!m_noMappedUpdate) {
        if (m_rangeMapper) {
            m_mappedValue = m_rangeMapper->getValueForPosition(value);
        } else {
            m_mappedValue = value;
        }
    }

    if (m_showTooltip) {
        QString name = objectName();
        QString unit = "";
        QString text;
        if (m_rangeMapper) unit = m_rangeMapper->getUnit();
        if (name != "") {
            text = tr("%1: %2%3").arg(name).arg(m_mappedValue).arg(unit);
        } else {
            text = tr("%2%3").arg(m_mappedValue).arg(unit);
        }
        setToolTip(text);
    }
}

void
AudioDial::setToDefault()
{
    if (m_rangeMapper) {
        setMappedValue(m_defaultMappedValue);
        return;
    }
    int dv = m_defaultValue;
    if (dv < minimum()) dv = minimum();
    if (dv > maximum()) dv = maximum();
    setValue(m_defaultValue);
}

// Alternate mouse behavior event handlers.
void AudioDial::mousePressEvent(QMouseEvent *mouseEvent)
{
    if (m_mouseDial) {
	QDial::mousePressEvent(mouseEvent);
    } else if (mouseEvent->button() == Qt::MidButton ||
               ((mouseEvent->button() == Qt::LeftButton) &&
                (mouseEvent->modifiers() & Qt::ControlModifier))) {
        setToDefault();
    } else if (mouseEvent->button() == Qt::LeftButton) {
	m_mousePressed = true;
	m_posMouse = mouseEvent->pos();
    }
}


void AudioDial::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
    //!!! needs a common base class with Thumbwheel

    if (m_mouseDial) {
	QDial::mouseDoubleClickEvent(mouseEvent);
    } else if (mouseEvent->button() != Qt::LeftButton) {
        return;
    }

    bool ok = false;

    if (m_rangeMapper) {
        
        double min = m_rangeMapper->getValueForPosition(minimum());
        double max = m_rangeMapper->getValueForPosition(maximum());
        
        if (min > max) { 
            double tmp = min;
            min = max;
            max = tmp;
        }

        QString unit = m_rangeMapper->getUnit();
        
        QString text;
        if (objectName() != "") {
            if (unit != "") {
                text = tr("New value for %1, from %2 to %3 %4:")
                    .arg(objectName()).arg(min).arg(max).arg(unit);
            } else {
                text = tr("New value for %1, from %2 to %3:")
                    .arg(objectName()).arg(min).arg(max);
            }
        } else {
            if (unit != "") {
                text = tr("Enter a new value from %1 to %2 %3:")
                    .arg(min).arg(max).arg(unit);
            } else {
                text = tr("Enter a new value from %1 to %2:")
                    .arg(min).arg(max);
            }
        }
        
        double newValue = QInputDialog::getDouble
            (this,
             tr("Enter new value"),
             text,
             m_mappedValue,
             min,
             max,
             4, 
             &ok);
        
        if (ok) {
            setMappedValue(newValue);
        }
        
    } else {
        
        int newPosition = QInputDialog::getInt
            (this,
             tr("Enter new value"),
             tr("Enter a new value from %1 to %2:")
             .arg(minimum()).arg(maximum()),
             value(), minimum(), maximum(), singleStep(), &ok);
        
        if (ok) {
            setValue(newPosition);
        }
    }
}


void AudioDial::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    if (m_mouseDial) {
	QDial::mouseMoveEvent(mouseEvent);
    } else if (m_mousePressed) {
	const QPoint& posMouse = mouseEvent->pos();
	int v = QDial::value()
	    + (posMouse.x() - m_posMouse.x())
	    + (m_posMouse.y() - posMouse.y());
	if (v > QDial::maximum())
	    v = QDial::maximum();
	else
	    if (v < QDial::minimum())
		v = QDial::minimum();
	m_posMouse = posMouse;
	QDial::setValue(v);
    }
}


void AudioDial::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (m_mouseDial) {
	QDial::mouseReleaseEvent(mouseEvent);
    } else if (m_mousePressed) {
	m_mousePressed = false;
    }
}

void
AudioDial::enterEvent(QEvent *e)
{
    QDial::enterEvent(e);
    emit mouseEntered();
}

void
AudioDial::leaveEvent(QEvent *e)
{
    QDial::enterEvent(e);
    emit mouseLeft();
}

