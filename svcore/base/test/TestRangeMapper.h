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

#ifndef TEST_RANGE_MAPPER_H
#define TEST_RANGE_MAPPER_H

#include "../RangeMapper.h"

#include <QObject>
#include <QtTest>
#include <QDir>

#include <iostream>

using namespace std;

class TestRangeMapper : public QObject
{
    Q_OBJECT

private slots:
    void linearUpForward()
    {
	LinearRangeMapper rm(1, 8, 0.5, 4.0, "x", false);
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getPositionForValue(0.5), 1);
	QCOMPARE(rm.getPositionForValue(4.0), 8);
	QCOMPARE(rm.getPositionForValue(3.0), 6);
	QCOMPARE(rm.getPositionForValue(3.1), 6);
	QCOMPARE(rm.getPositionForValue(3.4), 7);
	QCOMPARE(rm.getPositionForValue(0.2), 1);
	QCOMPARE(rm.getPositionForValue(-12), 1);
	QCOMPARE(rm.getPositionForValue(6.1), 8);
	QCOMPARE(rm.getPositionForValueUnclamped(3.0), 6);
	QCOMPARE(rm.getPositionForValueUnclamped(0.2), 0);
	QCOMPARE(rm.getPositionForValueUnclamped(-12), -24);
	QCOMPARE(rm.getPositionForValueUnclamped(6.1), 12);
    }

    void linearDownForward()
    {
	LinearRangeMapper rm(1, 8, 0.5, 4.0, "x", true);
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getPositionForValue(0.5), 8);
	QCOMPARE(rm.getPositionForValue(4.0), 1);
	QCOMPARE(rm.getPositionForValue(3.0), 3);
	QCOMPARE(rm.getPositionForValue(3.1), 3);
	QCOMPARE(rm.getPositionForValue(3.4), 2);
	QCOMPARE(rm.getPositionForValue(0.2), 8);
	QCOMPARE(rm.getPositionForValue(-12), 8);
	QCOMPARE(rm.getPositionForValue(6.1), 1);
	QCOMPARE(rm.getPositionForValueUnclamped(3.0), 3);
	QCOMPARE(rm.getPositionForValueUnclamped(0.2), 9);
	QCOMPARE(rm.getPositionForValueUnclamped(-12), 33);
	QCOMPARE(rm.getPositionForValueUnclamped(6.1), -3);
    }

    void linearUpBackward()
    {
	LinearRangeMapper rm(1, 8, 0.5, 4.0, "x", false);
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getValueForPosition(1), 0.5);
	QCOMPARE(rm.getValueForPosition(8), 4.0);
	QCOMPARE(rm.getValueForPosition(6), 3.0);
	QCOMPARE(rm.getValueForPosition(7), 3.5);
	QCOMPARE(rm.getValueForPosition(0), rm.getValueForPosition(1));
	QCOMPARE(rm.getValueForPosition(9), rm.getValueForPosition(8));
	QCOMPARE(rm.getValueForPositionUnclamped(6), 3.0);
	QCOMPARE(rm.getValueForPositionUnclamped(0), 0.0);
	QCOMPARE(rm.getValueForPositionUnclamped(-24), -12.0);
	QCOMPARE(rm.getValueForPositionUnclamped(12), 6.0);
    }

    void linearDownBackward()
    {
	LinearRangeMapper rm(1, 8, 0.5, 4.0, "x", true);
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getValueForPosition(8), 0.5);
	QCOMPARE(rm.getValueForPosition(1), 4.0);
	QCOMPARE(rm.getValueForPosition(3), 3.0);
	QCOMPARE(rm.getValueForPosition(2), 3.5);
	QCOMPARE(rm.getValueForPosition(0), rm.getValueForPosition(1));
	QCOMPARE(rm.getValueForPosition(9), rm.getValueForPosition(8));
	QCOMPARE(rm.getValueForPositionUnclamped(3), 3.0);
	QCOMPARE(rm.getValueForPositionUnclamped(9), 0.0);
	QCOMPARE(rm.getValueForPositionUnclamped(33), -12.0);
	QCOMPARE(rm.getValueForPositionUnclamped(-3), 6.0);
    }

    void logUpForward()
    {
	LogRangeMapper rm(3, 7, 10, 100000, "x", false);
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getPositionForValue(10.0), 3);
	QCOMPARE(rm.getPositionForValue(100000.0), 7);
	QCOMPARE(rm.getPositionForValue(1.0), 3);
	QCOMPARE(rm.getPositionForValue(1000000.0), 7);
	QCOMPARE(rm.getPositionForValue(1000.0), 5);
	QCOMPARE(rm.getPositionForValue(900.0), 5);
	QCOMPARE(rm.getPositionForValue(20000), 6);
	QCOMPARE(rm.getPositionForValueUnclamped(1.0), 2);
	QCOMPARE(rm.getPositionForValueUnclamped(1000000.0), 8);
	QCOMPARE(rm.getPositionForValueUnclamped(1000.0), 5);
    }

    void logDownForward()
    {
	LogRangeMapper rm(3, 7, 10, 100000, "x", true);
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getPositionForValue(10.0), 7);
	QCOMPARE(rm.getPositionForValue(100000.0), 3);
	QCOMPARE(rm.getPositionForValue(1.0), 7);
	QCOMPARE(rm.getPositionForValue(1000000.0), 3);
	QCOMPARE(rm.getPositionForValue(1000.0), 5);
	QCOMPARE(rm.getPositionForValue(900.0), 5);
	QCOMPARE(rm.getPositionForValue(20000), 4);
	QCOMPARE(rm.getPositionForValueUnclamped(1.0), 8);
	QCOMPARE(rm.getPositionForValueUnclamped(1000000.0), 2);
	QCOMPARE(rm.getPositionForValueUnclamped(1000.0), 5);
    }

    void logUpBackward()
    {
	LogRangeMapper rm(3, 7, 10, 100000, "x", false);
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getValueForPosition(3), 10.0);
	QCOMPARE(rm.getValueForPosition(7), 100000.0);
	QCOMPARE(rm.getValueForPosition(5), 1000.0);
	QCOMPARE(rm.getValueForPosition(6), 10000.0);
	QCOMPARE(rm.getValueForPosition(0), rm.getValueForPosition(3));
	QCOMPARE(rm.getValueForPosition(9), rm.getValueForPosition(7));
	QCOMPARE(rm.getValueForPositionUnclamped(2), 1.0);
        QCOMPARE(rm.getValueForPositionUnclamped(8), 1000000.0);
        QCOMPARE(rm.getValueForPositionUnclamped(5), 1000.0);
    }

    void logDownBackward()
    {
	LogRangeMapper rm(3, 7, 10, 100000, "x", true);
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getValueForPosition(7), 10.0);
	QCOMPARE(rm.getValueForPosition(3), 100000.0);
	QCOMPARE(rm.getValueForPosition(5), 1000.0);
	QCOMPARE(rm.getValueForPosition(4), 10000.0);
	QCOMPARE(rm.getValueForPosition(0), rm.getValueForPosition(3));
	QCOMPARE(rm.getValueForPosition(9), rm.getValueForPosition(7));
	QCOMPARE(rm.getValueForPositionUnclamped(8), 1.0);
        QCOMPARE(rm.getValueForPositionUnclamped(2), 1000000.0);
        QCOMPARE(rm.getValueForPositionUnclamped(5), 1000.0);
    }

    void interpolatingForward()
    {
	InterpolatingRangeMapper::CoordMap mappings;
	mappings[1] = 10;
	mappings[3] = 30;
	mappings[5] = 70;
	InterpolatingRangeMapper rm(mappings, "x");
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getPositionForValue(1.0), 10);
	QCOMPARE(rm.getPositionForValue(0.0), 10);
	QCOMPARE(rm.getPositionForValue(5.0), 70);
	QCOMPARE(rm.getPositionForValue(6.0), 70);
	QCOMPARE(rm.getPositionForValue(3.0), 30);
	QCOMPARE(rm.getPositionForValue(2.5), 25);
	QCOMPARE(rm.getPositionForValue(4.5), 60);
	QCOMPARE(rm.getPositionForValueUnclamped(0.0), 0);
	QCOMPARE(rm.getPositionForValueUnclamped(2.5), 25);
	QCOMPARE(rm.getPositionForValueUnclamped(6.0), 90);
    }

    void interpolatingBackward()
    {
	InterpolatingRangeMapper::CoordMap mappings;
	mappings[1] = 10;
	mappings[3] = 30;
	mappings[5] = 70;
	InterpolatingRangeMapper rm(mappings, "x");
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getValueForPosition(10), 1.0);
	QCOMPARE(rm.getValueForPosition(9), 1.0);
	QCOMPARE(rm.getValueForPosition(70), 5.0);
	QCOMPARE(rm.getValueForPosition(80), 5.0);
	QCOMPARE(rm.getValueForPosition(30), 3.0);
	QCOMPARE(rm.getValueForPosition(25), 2.5);
	QCOMPARE(rm.getValueForPosition(60), 4.5);
    }

    void autoLinearForward()
    {
	AutoRangeMapper::CoordMap mappings;
	mappings[0.5] = 1;
	mappings[4.0] = 8;
	AutoRangeMapper rm1(mappings, "x");
	QCOMPARE(rm1.getUnit(), QString("x"));
	QCOMPARE(rm1.getType(), AutoRangeMapper::StraightLine);
	QCOMPARE(rm1.getPositionForValue(0.1), 1);
	QCOMPARE(rm1.getPositionForValue(0.5), 1);
	QCOMPARE(rm1.getPositionForValue(4.0), 8);
	QCOMPARE(rm1.getPositionForValue(4.5), 8);
	QCOMPARE(rm1.getPositionForValue(3.0), 6);
	QCOMPARE(rm1.getPositionForValue(3.1), 6);
	QCOMPARE(rm1.getPositionForValueUnclamped(0.1), 0);
	QCOMPARE(rm1.getPositionForValueUnclamped(3.1), 6);
	QCOMPARE(rm1.getPositionForValueUnclamped(4.5), 9);
	mappings[3.0] = 6;
	mappings[3.5] = 7;
	AutoRangeMapper rm2(mappings, "x");
	QCOMPARE(rm2.getUnit(), QString("x"));
	QCOMPARE(rm2.getType(), AutoRangeMapper::StraightLine);
	QCOMPARE(rm2.getPositionForValue(0.5), 1);
	QCOMPARE(rm2.getPositionForValue(4.0), 8);
	QCOMPARE(rm2.getPositionForValue(3.0), 6);
	QCOMPARE(rm2.getPositionForValue(3.1), 6);
    }

    void autoLogForward()
    {
	AutoRangeMapper::CoordMap mappings;
	mappings[10] = 3;
	mappings[1000] = 5;
	mappings[100000] = 7;
	AutoRangeMapper rm1(mappings, "x");
	QCOMPARE(rm1.getUnit(), QString("x"));
	QCOMPARE(rm1.getType(), AutoRangeMapper::Logarithmic);
	QCOMPARE(rm1.getPositionForValue(10.0), 3);
	QCOMPARE(rm1.getPositionForValue(100000.0), 7);
	QCOMPARE(rm1.getPositionForValue(1.0), 3);
	QCOMPARE(rm1.getPositionForValue(1000000.0), 7);
	QCOMPARE(rm1.getPositionForValue(1000.0), 5);
	QCOMPARE(rm1.getPositionForValue(900.0), 5);
	QCOMPARE(rm1.getPositionForValue(20000), 6);
	QCOMPARE(rm1.getPositionForValueUnclamped(1.0), 2);
	QCOMPARE(rm1.getPositionForValueUnclamped(900.0), 5);
	QCOMPARE(rm1.getPositionForValueUnclamped(1000000.0), 8);
	mappings[100] = 4;
	AutoRangeMapper rm2(mappings, "x");
	QCOMPARE(rm2.getUnit(), QString("x"));
	QCOMPARE(rm2.getType(), AutoRangeMapper::Logarithmic);
	QCOMPARE(rm2.getPositionForValue(10.0), 3);
	QCOMPARE(rm2.getPositionForValue(100000.0), 7);
	QCOMPARE(rm2.getPositionForValue(1.0), 3);
	QCOMPARE(rm2.getPositionForValue(1000000.0), 7);
	QCOMPARE(rm2.getPositionForValue(1000.0), 5);
	QCOMPARE(rm2.getPositionForValue(900.0), 5);
	QCOMPARE(rm2.getPositionForValue(20000), 6);
    }

    void autoInterpolatingForward()
    {
	AutoRangeMapper::CoordMap mappings;
	mappings[1] = 10;
	mappings[3] = 30;
	mappings[5] = 70;
	AutoRangeMapper rm(mappings, "x");
	QCOMPARE(rm.getUnit(), QString("x"));
	QCOMPARE(rm.getType(), AutoRangeMapper::Interpolating);
	QCOMPARE(rm.getPositionForValue(1.0), 10);
	QCOMPARE(rm.getPositionForValue(0.0), 10);
	QCOMPARE(rm.getPositionForValue(5.0), 70);
	QCOMPARE(rm.getPositionForValue(6.0), 70);
	QCOMPARE(rm.getPositionForValue(3.0), 30);
	QCOMPARE(rm.getPositionForValue(2.5), 25);
	QCOMPARE(rm.getPositionForValue(4.5), 60);
	QCOMPARE(rm.getPositionForValueUnclamped(0.0), 0);
	QCOMPARE(rm.getPositionForValueUnclamped(5.0), 70);
	QCOMPARE(rm.getPositionForValueUnclamped(6.0), 90);
    }
};

#endif


