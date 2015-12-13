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
   This is a modified version of a source file from the
   Rosegarden MIDI and audio sequencer and notation editor.
   This file copyright 2000-2006 Chris Cannam.
*/

#include <iostream>

#include <cstdlib>
#include <sstream>

using std::cerr;
using std::endl;

#include "RealTime.h"
#include "sys/time.h"

// #include "Preferences.h"

// A RealTime consists of two ints that must be at least 32 bits each.
// A signed 32-bit int can store values exceeding +/- 2 billion.  This
// means we can safely use our lower int for nanoseconds, as there are
// 1 billion nanoseconds in a second and we need to handle double that
// because of the implementations of addition etc that we use.
//
// The maximum valid RealTime on a 32-bit system is somewhere around
// 68 years: 999999999 nanoseconds longer than the classic Unix epoch.

#define ONE_BILLION 1000000000

RealTime::RealTime(int s, int n) :
    sec(s), nsec(n)
{
    if (sec == 0) {
        while (nsec <= -ONE_BILLION) { nsec += ONE_BILLION; --sec; }
        while (nsec >=  ONE_BILLION) { nsec -= ONE_BILLION; ++sec; }
    } else if (sec < 0) {
        while (nsec <= -ONE_BILLION) { nsec += ONE_BILLION; --sec; }
        while (nsec > 0)             { nsec -= ONE_BILLION; ++sec; }
    } else {
        while (nsec >=  ONE_BILLION) { nsec -= ONE_BILLION; ++sec; }
        while (nsec < 0)             { nsec += ONE_BILLION; --sec; }
    }
}

RealTime
RealTime::fromSeconds(double sec)
{
    if (sec >= 0) {
        return RealTime(int(sec), int((sec - int(sec)) * ONE_BILLION + 0.5));
    } else {
        return -fromSeconds(-sec);
    }
}

RealTime
RealTime::fromMilliseconds(int msec)
{
    return RealTime(msec / 1000, (msec % 1000) * 1000000);
}

RealTime
RealTime::fromNanoseconds(long long nsec)
{
    long long s = nsec / ONE_BILLION;
    long long ns = nsec - (s * ONE_BILLION);
    return RealTime(s, ns);
//    int sec = 0; long long _nsec = nsec;
//    while (_nsec <= -ONE_BILLION) { _nsec += ONE_BILLION; --sec; }
//    while (_nsec >=  ONE_BILLION) { _nsec -= ONE_BILLION; ++sec; }
//    return RealTime(sec, _nsec);
}

RealTime
RealTime::fromTimeval(const struct timeval &tv)
{
    return RealTime(tv.tv_sec, tv.tv_usec * 1000);
}

RealTime
RealTime::fromXsdDuration(std::string xsdd)
{
    RealTime t;

    int year = 0, month = 0, day = 0, hour = 0, minute = 0;
    double second = 0.0;

    int i = 0;

    const char *s = xsdd.c_str();
    int len = int(xsdd.length());

    bool negative = false, afterT = false;

    while (i < len) {

        if (s[i] == '-') {
            if (i == 0) negative = true;
            ++i;
            continue;
        }

        double value = 0.0;
        char *eptr = 0;

        if (isdigit(s[i]) || s[i] == '.') {
            value = strtod(&s[i], &eptr);
            i = int(eptr - s);
        }

        if (i == len) break;

        switch (s[i]) {
        case 'Y': year = int(value + 0.1); break;
        case 'D': day  = int(value + 0.1); break;
        case 'H': hour = int(value + 0.1); break;
        case 'M':
            if (afterT) minute = int(value + 0.1);
            else month = int(value + 0.1);
            break;
        case 'S':
            second = value;
            break;
        case 'T': afterT = true; break;
        };

        ++i;
    }

    if (year > 0) {
        std::cerr << "WARNING: This xsd:duration (\"" << xsdd << "\") contains a non-zero year.\nWith no origin and a limited data size, I will treat a year as exactly 31556952\nseconds and you should expect overflow and/or poor results." << std::endl;
        t = t + RealTime(year * 31556952, 0);
    }

    if (month > 0) {
        std::cerr << "WARNING: This xsd:duration (\"" << xsdd << "\") contains a non-zero month.\nWith no origin and a limited data size, I will treat a month as exactly 2629746\nseconds and you should expect overflow and/or poor results." << std::endl;
        t = t + RealTime(month * 2629746, 0);
    }

    if (day > 0) {
        t = t + RealTime(day * 86400, 0);
    }

    if (hour > 0) {
        t = t + RealTime(hour * 3600, 0);
    }

    if (minute > 0) {
        t = t + RealTime(minute * 60, 0);
    }

    t = t + fromSeconds(second);
    if (negative) {
        return -t;
    } else {
        return t;
    }
}

double
RealTime::toDouble() const
{
    double d = sec;
    d += double(nsec) / double(ONE_BILLION);
    return d;
}

std::ostream &operator<<(std::ostream &out, const RealTime &rt)
{
    if (rt < RealTime::zeroTime) {
        out << "-";
    } else {
        out << " ";
    }

    int s = (rt.sec < 0 ? -rt.sec : rt.sec);
    int n = (rt.nsec < 0 ? -rt.nsec : rt.nsec);

    out << s << ".";

    int nn(n);
    if (nn == 0) out << "00000000";
    else while (nn < (ONE_BILLION / 10)) {
        out << "0";
        nn *= 10;
    }
    
    out << n << "R";
    return out;
}

std::string
RealTime::toString(bool align) const
{
    std::stringstream out;
    out << *this;
    
    std::string s = out.str();

    if (!align && *this >= RealTime::zeroTime) {
        // remove leading " "
        s = s.substr(1, s.length() - 1);
    }

    // remove trailing R
    return s.substr(0, s.length() - 1);
}

RealTime
RealTime::fromString(std::string s)
{
    bool negative = false;
    int section = 0;
    std::string ssec, snsec;

    for (size_t i = 0; i < s.length(); ++i) {

        char c = s[i];
        if (isspace(c)) continue;

        if (section == 0) {

            if (c == '-') negative = true;
            else if (isdigit(c)) { section = 1; ssec += c; }
            else if (c == '.') section = 2;
            else break;

        } else if (section == 1) {

            if (c == '.') section = 2;
            else if (isdigit(c)) ssec += c;
            else break;

        } else if (section == 2) {

            if (isdigit(c)) snsec += c;
            else break;
        }
    }

    while (snsec.length() < 8) snsec += '0';

    int sec = atoi(ssec.c_str());
    int nsec = atoi(snsec.c_str());
    if (negative) sec = -sec;

    //    cerr << "RealTime::fromString: string " << s << " -> "
    //              << sec << " sec, " << nsec << " nsec" << endl;

    return RealTime(sec, nsec);
}

std::string
RealTime::toText(bool fixedDp) const
{
    if (*this < RealTime::zeroTime) return "-" + (-*this).toText(fixedDp);

    // REMOVE TO AVOID TIGHT COUPLING WITH SONIC VISUALISER
    //    Preferences *p = Preferences::getInstance();
    //    if (p) {
    //        int fps = 0;
    //        switch (p->getTimeToTextMode()) {
    //        case Preferences::TimeToTextMs: break;
    //        case Preferences::TimeToTextUs: fps = 1000000; break;
    //        case Preferences::TimeToText24Frame: fps = 24; break;
    //        case Preferences::TimeToText25Frame: fps = 25; break;
    //        case Preferences::TimeToText30Frame: fps = 30; break;
    //        case Preferences::TimeToText50Frame: fps = 50; break;
    //        case Preferences::TimeToText60Frame: fps = 60; break;
    //        }
    //        if (fps != 0) return toFrameText(fps);
    //    }

    std::stringstream out;

    if (sec >= 3600) {
        out << (sec / 3600) << ":";
    }

    if (sec >= 60) {
        out << (sec % 3600) / 60 << ":";
    }

    if (sec >= 10) {
        out << ((sec % 60) / 10);
    }

    out << (sec % 10);
    
    int ms = msec();

    if (ms != 0) {
        out << ".";
        out << (ms / 100);
        ms = ms % 100;
        if (ms != 0) {
            out << (ms / 10);
            ms = ms % 10;
        } else if (fixedDp) {
            out << "0";
        }
        if (ms != 0) {
            out << ms;
        } else if (fixedDp) {
            out << "0";
        }
    } else if (fixedDp) {
        out << ".000";
    }

    std::string s = out.str();

    return s;
}

std::string
RealTime::toFrameText(int fps) const
{
    if (*this < RealTime::zeroTime) return "-" + (-*this).toFrameText(fps);

    std::stringstream out;

    if (sec >= 3600) {
        out << (sec / 3600) << ":";
    }

    if (sec >= 60) {
        out << (sec % 3600) / 60 << ":";
    }

    if (sec >= 10) {
        out << ((sec % 60) / 10);
    }

    out << (sec % 10);
    
    int f = nsec / (ONE_BILLION / fps);

    int div = 1;
    int n = fps - 1;
    while ((n = n / 10)) {
        div *= 10;
    }

    out << ":";

    //    std::cerr << "div = " << div << ", f =  "<< f << std::endl;

    while (div) {
        int d = (f / div) % 10;
        out << d;
        div /= 10;
    }

    std::string s = out.str();

    //    std::cerr << "converted " << toString() << " to " << s << std::endl;

    return s;
}

std::string
RealTime::toSecText() const
{
    if (*this < RealTime::zeroTime) return "-" + (-*this).toSecText();

    std::stringstream out;

    if (sec >= 3600) {
        out << (sec / 3600) << ":";
    }

    if (sec >= 60) {
        out << (sec % 3600) / 60 << ":";
    }

    if (sec >= 10) {
        out << ((sec % 60) / 10);
    }

    out << (sec % 10);
    
    if (sec < 60) {
        out << "s";
    }

    std::string s = out.str();

    return s;
}

std::string
RealTime::toXsdDuration() const
{
    std::string s = "PT" + toString(false) + "S";
    return s;
}

RealTime
RealTime::operator*(int m) const
{
    double t = (double(nsec) / ONE_BILLION) * m;
    t += sec * m;
    return fromSeconds(t);
}

RealTime
RealTime::operator/(int d) const
{
    int secdiv = sec / d;
    int secrem = sec % d;

    double nsecdiv = (double(nsec) + ONE_BILLION * double(secrem)) / d;
    
    return RealTime(secdiv, int(nsecdiv + 0.5));
}

RealTime
RealTime::operator*(double m) const
{
    double t = (double(nsec) / ONE_BILLION) * m;
    t += sec * m;
    return fromSeconds(t);
}

RealTime
RealTime::operator/(double d) const
{
    double t = (double(nsec) / ONE_BILLION) / d;
    t += sec / d;
    return fromSeconds(t);
}

double 
RealTime::operator/(const RealTime &r) const
{
    double lTotal = double(sec) * ONE_BILLION + double(nsec);
    double rTotal = double(r.sec) * ONE_BILLION + double(r.nsec);
    
    if (rTotal == 0) return 0.0;
    else return lTotal/rTotal;
}

static RealTime
frame2RealTime_i(sv_frame_t frame, sv_frame_t iSampleRate)
{
    if (frame < 0) return -frame2RealTime_i(-frame, iSampleRate);

    RealTime rt;
    sv_frame_t sec = frame / iSampleRate;
    rt.sec = int(sec);
    frame -= sec * iSampleRate;
    rt.nsec = (int)(((double(frame) * 1000000.0) / double(iSampleRate)) * 1000.0);
    return rt;
}

sv_frame_t
RealTime::realTime2Frame(const RealTime &time, sv_samplerate_t sampleRate)
{
    if (time < zeroTime) return -realTime2Frame(-time, sampleRate);
    double s = time.sec + double(time.nsec + 1) / 1000000000.0;
    return sv_frame_t(s * sampleRate);
}

RealTime
RealTime::frame2RealTime(sv_frame_t frame, sv_samplerate_t sampleRate)
{
    if (sampleRate == double(int(sampleRate))) {
        return frame2RealTime_i(frame, int(sampleRate));
    }

    double sec = double(frame) / sampleRate;
    return fromSeconds(sec);
}

RealTime
RealTime::abs(const RealTime x)
{
    if (x < RealTime(0, 0)) return -x; else return x;
}

const RealTime RealTime::zeroTime(0,0);
