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
   This file copyright 2000-2006 Chris Cannam, Guillaume Laurent,
   and QMUL.
*/


#ifndef _PROFILER_H_
#define _PROFILER_H_

#include "system/System.h"

#include <ctime>
#include <sys/time.h>
#include <map>

#include "PraalineCore/Base/RealTime.h"

//#define NO_TIMING 1

//#define WANT_TIMING 1

#ifdef NDEBUG
#ifndef WANT_TIMING
#define NO_TIMING 1
#endif
#endif

/**
 * Profiling classes
 */

/**
 * The class holding all profiling data
 *
 * This class is a singleton
 */
class Profiles
{
public:
    static Profiles* getInstance();
    ~Profiles();

    void accumulate(const char* id, clock_t time, RealTime rt);
    void dump() const;

protected:
    Profiles();

    typedef std::pair<clock_t, RealTime> TimePair;
    typedef std::pair<int, TimePair> ProfilePair;
    typedef std::map<const char *, ProfilePair> ProfileMap;
    typedef std::map<const char *, TimePair> LastCallMap;
    typedef std::map<const char *, TimePair> WorstCallMap;
    ProfileMap m_profiles;
    LastCallMap m_lastCalls;
    WorstCallMap m_worstCalls;

    static Profiles* m_instance;
};

#ifndef NO_TIMING

/**
 * Profile point instance class.  Construct one of these on the stack
 * at the start of a function, in order to record the time consumed
 * within that function.  The profiler object should be effectively
 * optimised out if NO_TIMING is defined, so any overhead in a release
 * build should be negligible so long as you remember to define that.
 */
class Profiler
{
public:
    /**
     * Create a profile point instance that records time consumed
     * against the given profiling point name.  If showOnDestruct is
     * true, the time consumed will be printed to stderr when the
     * object is destroyed; otherwise, only the accumulated, mean and
     * worst-case times will be shown when the program exits or
     * Profiles::dump() is called.
     */
    Profiler(const char *name, bool showOnDestruct = false);
    ~Profiler();

    void update() const;
    void end(); // same action as dtor

protected:
    const char* m_c;
    clock_t m_startCPU;
    RealTime m_startTime;
    bool m_showOnDestruct;
    bool m_ended;
};

#else

class Profiler
{
public:
    Profiler(const char *, bool = false) { }
    ~Profiler() { }

    void update() const { }
    void end() { }
};

#endif

#endif
