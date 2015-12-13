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

#ifndef _THREAD_H_
#define _THREAD_H_

#include <QThread>
#include <QMutexLocker>

#include "Profiler.h"

class Thread : public QThread
{
Q_OBJECT

public:
    enum Type { RTThread, NonRTThread };

    Thread(Type type = NonRTThread, QObject *parent = 0);

public slots:
    void start();

protected:
    virtual void run() = 0;

private:    
    Type m_type;
};

class MutexLocker
{
public:
    MutexLocker(QMutex *mutex, const char *name);
    ~MutexLocker();

private:
    class Printer {
    public:
        Printer(const char *name);
        ~Printer();

    private:
        const char *m_name;
    };

    Profiler m_profiler;
    Printer m_printer;
    QMutexLocker m_locker;
};

#endif
