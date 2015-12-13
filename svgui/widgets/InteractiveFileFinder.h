/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _INTERACTIVE_FILE_FINDER_H_
#define _INTERACTIVE_FILE_FINDER_H_

#include "data/fileio/FileFinder.h"

#include <QApplication>
#include <QString>
#include <QObject>

class InteractiveFileFinder : public QObject,
                              public FileFinder
{
    Q_OBJECT

public:
    virtual ~InteractiveFileFinder();

    /// Specify the extension for this application's session files
    /// (without the dot)
    void setApplicationSessionExtension(QString extension);

    QString getApplicationSessionExtension() const {
        return m_sessionExtension;
    }

    QString getOpenFileName(FileType type, QString fallbackLocation = "");
    QString getSaveFileName(FileType type, QString fallbackLocation = "");
    void registerLastOpenedFilePath(FileType type, QString path);

    QString find(FileType type, QString location, QString lastKnownLocation = "");

    static void setParentWidget(QWidget *);

    static InteractiveFileFinder *getInstance() { return &m_instance; }

protected:
    InteractiveFileFinder();
    static InteractiveFileFinder m_instance;

    QString findRelative(QString location, QString relativeTo);
    QString locateInteractive(FileType type, QString thing);

    QString m_sessionExtension;
    QString m_lastLocatedLocation;

    QWidget *m_parent;
};

#endif

