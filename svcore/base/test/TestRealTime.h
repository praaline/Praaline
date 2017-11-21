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

#ifndef TEST_REALTIME_H
#define TEST_REALTIME_H

#include "../RealTime.h"

#include <QObject>
#include <QtTest>
#include <QDir>

#include <iostream>

using namespace std;

class TestRealTime : public QObject
{
    Q_OBJECT

private slots:

#define ONE_MILLION 1000000
#define ONE_BILLION 1000000000
    
    void zero()
    {
	QCOMPARE(RealTime(0, 0), RealTime::zeroTime);
	QCOMPARE(RealTime(0, 0).sec, 0);
	QCOMPARE(RealTime(0, 0).nsec, 0);
	QCOMPARE(RealTime(0, 0).msec(), 0);
	QCOMPARE(RealTime(0, 0).usec(), 0);
    }

    void ctor()
    {
	QCOMPARE(RealTime(0, 0), RealTime(0, 0));

	// wraparounds
	QCOMPARE(RealTime(0, ONE_BILLION/2), RealTime(1, -ONE_BILLION/2));
	QCOMPARE(RealTime(0, -ONE_BILLION/2), RealTime(-1, ONE_BILLION/2));

	QCOMPARE(RealTime(1, ONE_BILLION), RealTime(2, 0));
	QCOMPARE(RealTime(1, -ONE_BILLION), RealTime(0, 0));
	QCOMPARE(RealTime(-1, ONE_BILLION), RealTime(0, 0));
	QCOMPARE(RealTime(-1, -ONE_BILLION), RealTime(-2, 0));

	QCOMPARE(RealTime(2, -ONE_BILLION*2), RealTime(0, 0));
	QCOMPARE(RealTime(2, -ONE_BILLION/2), RealTime(1, ONE_BILLION/2));

	QCOMPARE(RealTime(-2, ONE_BILLION*2), RealTime(0, 0));
	QCOMPARE(RealTime(-2, ONE_BILLION/2), RealTime(-1, -ONE_BILLION/2));
	
	QCOMPARE(RealTime(0, 1).sec, 0);
	QCOMPARE(RealTime(0, 1).nsec, 1);
	QCOMPARE(RealTime(0, -1).sec, 0);
	QCOMPARE(RealTime(0, -1).nsec, -1);
	QCOMPARE(RealTime(1, -1).sec, 0);
	QCOMPARE(RealTime(1, -1).nsec, ONE_BILLION-1);
	QCOMPARE(RealTime(-1, 1).sec, 0);
	QCOMPARE(RealTime(-1, 1).nsec, -ONE_BILLION+1);
	QCOMPARE(RealTime(-1, -1).sec, -1);
	QCOMPARE(RealTime(-1, -1).nsec, -1);
	
	QCOMPARE(RealTime(2, -ONE_BILLION*2).sec, 0);
	QCOMPARE(RealTime(2, -ONE_BILLION*2).nsec, 0);
	QCOMPARE(RealTime(2, -ONE_BILLION/2).sec, 1);
	QCOMPARE(RealTime(2, -ONE_BILLION/2).nsec, ONE_BILLION/2);

	QCOMPARE(RealTime(-2, ONE_BILLION*2).sec, 0);
	QCOMPARE(RealTime(-2, ONE_BILLION*2).nsec, 0);
	QCOMPARE(RealTime(-2, ONE_BILLION/2).sec, -1);
	QCOMPARE(RealTime(-2, ONE_BILLION/2).nsec, -ONE_BILLION/2);
    }
    
    void fromSeconds()
    {
	QCOMPARE(RealTime::fromSeconds(0), RealTime(0, 0));

	QCOMPARE(RealTime::fromSeconds(0.5).sec, 0);
	QCOMPARE(RealTime::fromSeconds(0.5).nsec, ONE_BILLION/2);
	QCOMPARE(RealTime::fromSeconds(0.5).usec(), ONE_MILLION/2);
	QCOMPARE(RealTime::fromSeconds(0.5).msec(), 500);
	
	QCOMPARE(RealTime::fromSeconds(0.5), RealTime(0, ONE_BILLION/2));
	QCOMPARE(RealTime::fromSeconds(1), RealTime(1, 0));
	QCOMPARE(RealTime::fromSeconds(1.5), RealTime(1, ONE_BILLION/2));

	QCOMPARE(RealTime::fromSeconds(-0.5).sec, 0);
	QCOMPARE(RealTime::fromSeconds(-0.5).nsec, -ONE_BILLION/2);
	QCOMPARE(RealTime::fromSeconds(-0.5).usec(), -ONE_MILLION/2);
	QCOMPARE(RealTime::fromSeconds(-0.5).msec(), -500);
	
	QCOMPARE(RealTime::fromSeconds(-1.5).sec, -1);
	QCOMPARE(RealTime::fromSeconds(-1.5).nsec, -ONE_BILLION/2);
	QCOMPARE(RealTime::fromSeconds(-1.5).usec(), -ONE_MILLION/2);
	QCOMPARE(RealTime::fromSeconds(-1.5).msec(), -500);
	
	QCOMPARE(RealTime::fromSeconds(-0.5), RealTime(0, -ONE_BILLION/2));
	QCOMPARE(RealTime::fromSeconds(-1), RealTime(-1, 0));
	QCOMPARE(RealTime::fromSeconds(-1.5), RealTime(-1, -ONE_BILLION/2));
    }

    void fromMilliseconds()
    {
	QCOMPARE(RealTime::fromMilliseconds(0), RealTime(0, 0));
	QCOMPARE(RealTime::fromMilliseconds(500), RealTime(0, ONE_BILLION/2));
	QCOMPARE(RealTime::fromMilliseconds(1000), RealTime(1, 0));
	QCOMPARE(RealTime::fromMilliseconds(1500), RealTime(1, ONE_BILLION/2));

    	QCOMPARE(RealTime::fromMilliseconds(-0), RealTime(0, 0));
	QCOMPARE(RealTime::fromMilliseconds(-500), RealTime(0, -ONE_BILLION/2));
	QCOMPARE(RealTime::fromMilliseconds(-1000), RealTime(-1, 0));
	QCOMPARE(RealTime::fromMilliseconds(-1500), RealTime(-1, -ONE_BILLION/2));
    }
    
    void fromTimeval()
    {
	struct timeval tv;

	tv.tv_sec = 0; tv.tv_usec = 0;
	QCOMPARE(RealTime::fromTimeval(tv), RealTime(0, 0));
	tv.tv_sec = 0; tv.tv_usec = ONE_MILLION/2;
	QCOMPARE(RealTime::fromTimeval(tv), RealTime(0, ONE_BILLION/2));
	tv.tv_sec = 1; tv.tv_usec = 0;
	QCOMPARE(RealTime::fromTimeval(tv), RealTime(1, 0));
	tv.tv_sec = 1; tv.tv_usec = ONE_MILLION/2;
	QCOMPARE(RealTime::fromTimeval(tv), RealTime(1, ONE_BILLION/2));

	tv.tv_sec = 0; tv.tv_usec = -ONE_MILLION/2;
	QCOMPARE(RealTime::fromTimeval(tv), RealTime(0, -ONE_BILLION/2));
	tv.tv_sec = -1; tv.tv_usec = 0;
	QCOMPARE(RealTime::fromTimeval(tv), RealTime(-1, 0));
	tv.tv_sec = -1; tv.tv_usec = -ONE_MILLION/2;
	QCOMPARE(RealTime::fromTimeval(tv), RealTime(-1, -ONE_BILLION/2));
    }

    void fromXsdDuration()
    {
	QCOMPARE(RealTime::fromXsdDuration("PT0"), RealTime::zeroTime);
	QCOMPARE(RealTime::fromXsdDuration("PT0S"), RealTime::zeroTime);
	QCOMPARE(RealTime::fromXsdDuration("PT10S"), RealTime(10, 0));
	QCOMPARE(RealTime::fromXsdDuration("PT10.5S"), RealTime(10, ONE_BILLION/2));
	QCOMPARE(RealTime::fromXsdDuration("PT1.5S").sec, 1);
	QCOMPARE(RealTime::fromXsdDuration("PT1.5S").msec(), 500);
	QCOMPARE(RealTime::fromXsdDuration("-PT1.5S").sec, -1);
	QCOMPARE(RealTime::fromXsdDuration("-PT1.5S").msec(), -500);
	QCOMPARE(RealTime::fromXsdDuration("PT1M30.5S"), RealTime(90, ONE_BILLION/2));
	QCOMPARE(RealTime::fromXsdDuration("PT1H2M30.5S"), RealTime(3750, ONE_BILLION/2));
    }

    void toDouble()
    {
	QCOMPARE(RealTime(0, 0).toDouble(), 0.0);
	QCOMPARE(RealTime(0, ONE_BILLION/2).toDouble(), 0.5);
	QCOMPARE(RealTime(1, 0).toDouble(), 1.0);
	QCOMPARE(RealTime(1, ONE_BILLION/2).toDouble(), 1.5);

	QCOMPARE(RealTime(0, -ONE_BILLION/2).toDouble(), -0.5);
	QCOMPARE(RealTime(-1, 0).toDouble(), -1.0);
	QCOMPARE(RealTime(-1, -ONE_BILLION/2).toDouble(), -1.5);
    }

    void assign()
    {
	RealTime r;
	r = RealTime(0, 0);
	QCOMPARE(r, RealTime::zeroTime);
	r = RealTime(0, ONE_BILLION/2);
	QCOMPARE(r.toDouble(), 0.5);
	r = RealTime(-1, -ONE_BILLION/2);
	QCOMPARE(r.toDouble(), -1.5);
    }

    void plus()
    {
	QCOMPARE(RealTime(0, 0) + RealTime(0, 0), RealTime(0, 0));

	QCOMPARE(RealTime(0, 0) + RealTime(0, ONE_BILLION/2), RealTime(0, ONE_BILLION/2));
	QCOMPARE(RealTime(0, ONE_BILLION/2) + RealTime(0, ONE_BILLION/2), RealTime(1, 0));
	QCOMPARE(RealTime(1, 0) + RealTime(0, ONE_BILLION/2), RealTime(1, ONE_BILLION/2));

	QCOMPARE(RealTime(0, 0) + RealTime(0, -ONE_BILLION/2), RealTime(0, -ONE_BILLION/2));
	QCOMPARE(RealTime(0, -ONE_BILLION/2) + RealTime(0, -ONE_BILLION/2), RealTime(-1, 0));
	QCOMPARE(RealTime(-1, 0) + RealTime(0, -ONE_BILLION/2), RealTime(-1, -ONE_BILLION/2));

    	QCOMPARE(RealTime(1, 0) + RealTime(0, -ONE_BILLION/2), RealTime(0, ONE_BILLION/2));
	QCOMPARE(RealTime(1, 0) + RealTime(0, -ONE_BILLION/2) + RealTime(0, -ONE_BILLION/2), RealTime(0, 0));
	QCOMPARE(RealTime(1, 0) + RealTime(0, -ONE_BILLION/2) + RealTime(0, -ONE_BILLION/2) + RealTime(0, -ONE_BILLION/2), RealTime(0, -ONE_BILLION/2));

	QCOMPARE(RealTime(0, ONE_BILLION/2) + RealTime(-1, 0), RealTime(0, -ONE_BILLION/2));
	QCOMPARE(RealTime(0, -ONE_BILLION/2) + RealTime(1, 0), RealTime(0, ONE_BILLION/2));
    }
    
    void minus()
    {
	QCOMPARE(RealTime(0, 0) - RealTime(0, 0), RealTime(0, 0));

	QCOMPARE(RealTime(0, 0) - RealTime(0, ONE_BILLION/2), RealTime(0, -ONE_BILLION/2));
	QCOMPARE(RealTime(0, ONE_BILLION/2) - RealTime(0, ONE_BILLION/2), RealTime(0, 0));
	QCOMPARE(RealTime(1, 0) - RealTime(0, ONE_BILLION/2), RealTime(0, ONE_BILLION/2));

	QCOMPARE(RealTime(0, 0) - RealTime(0, -ONE_BILLION/2), RealTime(0, ONE_BILLION/2));
	QCOMPARE(RealTime(0, -ONE_BILLION/2) - RealTime(0, -ONE_BILLION/2), RealTime(0, 0));
	QCOMPARE(RealTime(-1, 0) - RealTime(0, -ONE_BILLION/2), RealTime(0, -ONE_BILLION/2));

    	QCOMPARE(RealTime(1, 0) - RealTime(0, -ONE_BILLION/2), RealTime(1, ONE_BILLION/2));
	QCOMPARE(RealTime(1, 0) - RealTime(0, -ONE_BILLION/2) - RealTime(0, -ONE_BILLION/2), RealTime(2, 0));
	QCOMPARE(RealTime(1, 0) - RealTime(0, -ONE_BILLION/2) - RealTime(0, -ONE_BILLION/2) - RealTime(0, -ONE_BILLION/2), RealTime(2, ONE_BILLION/2));

	QCOMPARE(RealTime(0, ONE_BILLION/2) - RealTime(-1, 0), RealTime(1, ONE_BILLION/2));
	QCOMPARE(RealTime(0, -ONE_BILLION/2) - RealTime(1, 0), RealTime(-1, -ONE_BILLION/2));
    }

    void negate()
    {
	QCOMPARE(-RealTime(0, 0), RealTime(0, 0));
	QCOMPARE(-RealTime(1, 0), RealTime(-1, 0));
	QCOMPARE(-RealTime(1, ONE_BILLION/2), RealTime(-1, -ONE_BILLION/2));
	QCOMPARE(-RealTime(-1, -ONE_BILLION/2), RealTime(1, ONE_BILLION/2));
    }

    void compare()
    {
	int sec, nsec;
	for (sec = -2; sec <= 2; sec += 2) {
	    for (nsec = -1; nsec <= 1; nsec += 1) {
		QCOMPARE(RealTime(sec, nsec) < RealTime(sec, nsec), false);
		QCOMPARE(RealTime(sec, nsec) > RealTime(sec, nsec), false);
		QCOMPARE(RealTime(sec, nsec) == RealTime(sec, nsec), true);
		QCOMPARE(RealTime(sec, nsec) != RealTime(sec, nsec), false);
		QCOMPARE(RealTime(sec, nsec) <= RealTime(sec, nsec), true);
		QCOMPARE(RealTime(sec, nsec) >= RealTime(sec, nsec), true);
	    }
	}
	RealTime prev(-3, 0);
	for (sec = -2; sec <= 2; sec += 2) {
	    for (nsec = -1; nsec <= 1; nsec += 1) {

		RealTime curr(sec, nsec);

		QCOMPARE(prev < curr, true);
		QCOMPARE(prev > curr, false);
		QCOMPARE(prev == curr, false);
		QCOMPARE(prev != curr, true);
		QCOMPARE(prev <= curr, true);
		QCOMPARE(prev >= curr, false);

		QCOMPARE(curr < prev, false);
		QCOMPARE(curr > prev, true);
		QCOMPARE(curr == prev, false);
		QCOMPARE(curr != prev, true);
		QCOMPARE(curr <= prev, false);
		QCOMPARE(curr >= prev, true);

		prev = curr;
	    }
	}
    }

    void frame()
    {
        int frames[] = {
            0, 1, 2047, 2048, 6656, 32767, 32768, 44100, 44101, 999999999
        };
        int n = sizeof(frames)/sizeof(frames[0]);

        int rates[] = {
            1, 2, 8000, 22050, 44100, 44101, 192000
        };
        int m = sizeof(rates)/sizeof(rates[0]);

        for (int i = 0; i < n; ++i) {
            sv_frame_t frame = frames[i];
            for (int j = 0; j < m; ++j) {
                int rate = rates[j];

                RealTime rt = RealTime::frame2RealTime(frame, rate);
                sv_frame_t conv = RealTime::realTime2Frame(rt, rate);
                QCOMPARE(frame, conv);

                rt = RealTime::frame2RealTime(-frame, rate);
                conv = RealTime::realTime2Frame(rt, rate);
                QCOMPARE(-frame, conv);
            }
        }
    }
};

#endif
