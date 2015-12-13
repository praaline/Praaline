/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _COLOUR_MAPPER_H_
#define _COLOUR_MAPPER_H_

#include <QObject>
#include <QColor>
#include <QString>

/**
 * A class for mapping intensity values onto various colour maps.
 */

class ColourMapper : public QObject
{
    Q_OBJECT

public:
    ColourMapper(int map, double minValue, double maxValue);
    virtual ~ColourMapper();

    enum StandardMap {
        DefaultColours,
        Sunset,
        WhiteOnBlack,
        BlackOnWhite,
        RedOnBlue,
        YellowOnBlack,
        BlueOnBlack,
        FruitSalad,
        Banded,
        Highlight,
        Printer,
        HighGain
    };

    int getMap() const { return m_map; }
    double getMinValue() const { return m_min; }
    double getMaxValue() const { return m_max; }

    static int getColourMapCount();
    static QString getColourMapName(int n);

    QColor map(double value) const;

    QColor getContrastingColour() const; // for cursors etc
    bool hasLightBackground() const;

protected:
    int m_map;
    double m_min;
    double m_max;
};

#endif

