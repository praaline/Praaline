/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007-2008 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _PROGRESS_REPORTER_H_
#define _PROGRESS_REPORTER_H_

#include <QObject>
#include <QString>

class ProgressReporter : public QObject
{
    Q_OBJECT
    
public:
    ProgressReporter(QObject *parent = nullptr);
    virtual ~ProgressReporter();

    virtual bool isDefinite() const = 0;
    virtual void setDefinite(bool definite) = 0; // default should be definite

    virtual bool wasCancelled() const = 0;

signals:
    void cancelled();

public slots:
    virtual void setMessage(QString text) = 0;
    virtual void setProgress(int percentage) = 0;
};

#endif
