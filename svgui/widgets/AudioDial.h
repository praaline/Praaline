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

#ifndef _AUDIO_DIAL_H_
#define _AUDIO_DIAL_H_

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

#include <QDial>
#include <map>

class RangeMapper;

/**
 * AudioDial is a nicer-looking QDial that by default reacts to mouse
 * movement on horizontal and vertical axes instead of in a radial
 * motion.  Move the mouse up or right to increment the value, down or
 * left to decrement it.  AudioDial also responds to the mouse wheel.
 *
 * The programming interface for this widget is compatible with QDial,
 * with the addition of properties for the knob colour and meter
 * colour and a boolean property mouseDial that determines whether to
 * respond to radial mouse motion in the same way as QDial (the
 * default is no).
 */

class AudioDial : public QDial
{
    Q_OBJECT
    Q_PROPERTY( QColor knobColor READ getKnobColor WRITE setKnobColor )
    Q_PROPERTY( QColor meterColor READ getMeterColor WRITE setMeterColor )
    Q_PROPERTY( bool mouseDial READ getMouseDial WRITE setMouseDial )

public:
    AudioDial(QWidget *parent = 0);
    ~AudioDial();

    const QColor& getKnobColor()  const { return m_knobColor;  }
    const QColor& getMeterColor() const { return m_meterColor; }
    bool getMouseDial() const { return m_mouseDial; }

    void setRangeMapper(RangeMapper *mapper); // I take ownership, will delete
    const RangeMapper *rangeMapper() const { return m_rangeMapper; }
    double mappedValue() const;

    int defaultValue() const { return m_defaultValue; }

    void setShowToolTip(bool show);

signals:
    void mouseEntered();
    void mouseLeft();

public slots:
    /**
     * Set the colour of the knob.  The default is to inherit the
     * colour from the widget's palette.
     */
    void setKnobColor(const QColor &color);

    /**
     * Set the colour of the meter (the highlighted area around the
     * knob that shows the current value).  The default is to inherit
     * the colour from the widget's palette.
     */
    void setMeterColor(const QColor &color);
    
    /**
     * Specify that the dial should respond to radial mouse movements
     * in the same way as QDial.
     */
    void setMouseDial(bool mouseDial);

    void setDefaultValue(int defaultValue);

    void setValue(int value);

    void setDefaultMappedValue(double mappedValue);

    void setMappedValue(double mappedValue);

    void setToDefault();

protected:
    void drawTick(QPainter &paint, double angle, int size, bool internal);
    virtual void paintEvent(QPaintEvent *);

    // Alternate mouse behavior event handlers.
    virtual void mousePressEvent(QMouseEvent *pMouseEvent);
    virtual void mouseMoveEvent(QMouseEvent *pMouseEvent);
    virtual void mouseReleaseEvent(QMouseEvent *pMouseEvent);
    virtual void mouseDoubleClickEvent(QMouseEvent *pMouseEvent);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

protected slots:
    void updateMappedValue(int value);

private:
    QColor m_knobColor;
    QColor m_meterColor;
    
    int m_defaultValue;
    double m_defaultMappedValue;
    double m_mappedValue;
    bool m_noMappedUpdate;

    // Alternate mouse behavior tracking.
    bool m_mouseDial;
    bool m_mousePressed;
    QPoint m_posMouse;

    bool m_showTooltip;

    RangeMapper *m_rangeMapper;
};


#endif  // __AudioDial_h

// end of AudioDial.h
