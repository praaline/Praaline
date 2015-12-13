/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _CACHED_FILE_H_
#define _CACHED_FILE_H_

#include <QString>
#include <QUrl>
#include <QDateTime>
#include <map>

class ProgressReporter;

class CachedFile
{
public:
    CachedFile(QString fileOrUrl,
               ProgressReporter *reporter = 0,
               QString preferredContentType = "");
    CachedFile(QUrl url,
               ProgressReporter *reporter = 0,
               QString preferredContentType = "");

    virtual ~CachedFile();

    bool isOK() const;

    QString getLocalFilename() const;

protected:
    QString m_origin;
    QString m_localFilename;
    QString m_preferredContentType;
    ProgressReporter *m_reporter;
    bool m_ok;

    void checkFile();
    bool retrieve();

    QDateTime getLastRetrieval();
    void updateLastRetrieval(bool successful);

    static QString getCacheDirectory();
    static QString getLocalFilenameFor(QUrl url);

    typedef std::map<QString, QString> OriginLocalFilenameMap;
    static OriginLocalFilenameMap m_knownGoodCaches;
};

#endif
