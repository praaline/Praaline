/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2010-2011 Chris Cannam and QMUL.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Debug.h"
#include "ResourceFinder.h"

#include <QString>
#include <QUrl>
#include <QMutex>
#include <QMutexLocker>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QDateTime>
#include <QThreadStorage>

#include <cstdio>

static QThreadStorage<QDebug *> debugs;
static QMutex mutex;
static char *prefix = 0;

QDebug &
getSVDebug()
{
    mutex.lock();

    QDebug *debug = 0;

    QString pfx = ResourceFinder().getUserResourcePrefix();
    QDir logdir(QString("%1/%2").arg(pfx).arg("log"));

    if (!prefix) {
        prefix = strdup(QString("[%1]")
                        .arg(QCoreApplication::applicationPid())
                        .toLatin1().data());
        //!!! what to do if mkpath fails?
    if (!logdir.exists()) logdir.mkpath(logdir.path());
    }

    if (!debugs.hasLocalData()) {
        QFile *logFile = new QFile(logdir.path() + "/sv-debug.log");
        if (logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
            QDebug(QtDebugMsg) << (const char *)prefix
                               << "Opened debug log file "
                               << logFile->fileName();
            debug = new QDebug(logFile);
        } else {
            QDebug(QtWarningMsg) << (const char *)prefix
                                 << "Failed to open debug log file "
                                 << logFile->fileName()
                                 << " for writing, using console debug instead";
            delete logFile;
            logFile = 0;
            debug = new QDebug(QtDebugMsg);
        }
        debugs.setLocalData(debug);
        *debug << Qt::endl << (const char *)prefix << "Log started at "
               << QDateTime::currentDateTime().toString();
    } else {
        debug = debugs.localData();
    }

    mutex.unlock();

    QDebug &dref = *debug;
    dref << Qt::endl << (const char *)prefix;

    return dref;
}

QDebug &
operator<<(QDebug &dbg, const std::string &s)
{
    dbg << QString::fromUtf8(s.c_str());
    return dbg;
}

std::ostream &
operator<<(std::ostream &target, const QString &str)
{
    return target << str.toStdString();
}

std::ostream &
operator<<(std::ostream &target, const QUrl &u)
{
    return target << "<" << u.toString().toStdString() << ">";
}

