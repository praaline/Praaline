/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Thread.h"

#ifndef _WIN32
#include <pthread.h>
#endif

//#define DEBUG_MUTEX_LOCKER 1

#include <iostream>

Thread::Thread(Type type, QObject *parent) :
    QThread(parent),
    m_type(type)
{
    setStackSize(512 * 1024);
}

void
Thread::start()
{
    QThread::start();

#ifndef _WIN32
    struct sched_param param;
    ::memset(&param, 0, sizeof(param));

    if (m_type == RTThread) {

        param.sched_priority = 5;

        if (::pthread_setschedparam(pthread_self(), SCHED_FIFO, &param)) {
            ::perror("INFO: pthread_setschedparam to SCHED_FIFO failed");
        }

    } else {

        if (::pthread_setschedparam(pthread_self(), SCHED_OTHER, &param)) {
            ::perror("WARNING: pthread_setschedparam to SCHED_OTHER failed");
        }
    }        

#endif
}

MutexLocker::MutexLocker(QMutex *mutex, const char *name) :
    m_profiler(name, false),
    m_printer(name),
    m_locker(mutex)
{
#ifdef DEBUG_MUTEX_LOCKER
    cerr << "... locked mutex " << mutex << endl;
#endif
    m_profiler.end();
}

MutexLocker::~MutexLocker()
{
}

MutexLocker::Printer::Printer(const char *name) :
    m_name(name)
{
#ifdef DEBUG_MUTEX_LOCKER
    cerr << "MutexLocker: Locking   \"" << m_name << "\" in "
              << (void *)QThread::currentThreadId() << endl;
#endif
}

MutexLocker::Printer::~Printer()
{
#ifdef DEBUG_MUTEX_LOCKER
    cerr << "MutexLocker: Unlocking \"" << m_name
              << "\" in " << (void *)QThread::currentThreadId() << endl;
#endif
}

