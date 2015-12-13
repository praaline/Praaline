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

#include "CachedFile.h"

#include "base/TempDirectory.h"
#include "base/ProgressReporter.h"
#include "base/Exceptions.h"

#include "FileSource.h"

#include <QFileInfo>
#include <QSettings>
#include <QVariant>
#include <QMap>
#include <QDir>
#include <QCryptographicHash>

#include "base/Profiler.h"

#include <iostream>

CachedFile::OriginLocalFilenameMap
CachedFile::m_knownGoodCaches;

QString
CachedFile::getLocalFilenameFor(QUrl url)
{
    Profiler p("CachedFile::getLocalFilenameFor");

    QDir dir(getCacheDirectory());

    QString filename =
        QString::fromLocal8Bit
        (QCryptographicHash::hash(url.toString().toLocal8Bit(),
                                  QCryptographicHash::Sha1).toHex());

    return dir.filePath(filename);
}

QString
CachedFile::getCacheDirectory()
{
    QDir dir = TempDirectory::getInstance()->getContainingPath();

    QString cacheDirName("cache");

    QFileInfo fi(dir.filePath(cacheDirName));

    if ((fi.exists() && !fi.isDir()) ||
        (!fi.exists() && !dir.mkdir(cacheDirName))) {

        throw DirectoryCreationFailed(fi.filePath());
    }

    return fi.filePath();
}

CachedFile::CachedFile(QString origin,
                       ProgressReporter *reporter,
                       QString preferredContentType) :
    m_origin(origin),
    m_preferredContentType(preferredContentType),
    m_reporter(reporter),
    m_ok(false)
{
    Profiler p("CachedFile::CachedFile[1]");

    cerr << "CachedFile::CachedFile: origin is \""
              << origin << "\"" << endl;
    checkFile();
}

CachedFile::CachedFile(QUrl url,
                       ProgressReporter *reporter,
    QString preferredContentType) :
    m_origin(url.toString()),
    m_preferredContentType(preferredContentType),
    m_reporter(reporter),
    m_ok(false)
{
    Profiler p("CachedFile::CachedFile[2]");

    cerr << "CachedFile::CachedFile: url is \""
              << url.toString() << "\"" << endl;
    checkFile();
}

CachedFile::~CachedFile()
{
}

bool
CachedFile::isOK() const
{
    return m_ok;
}

QString
CachedFile::getLocalFilename() const
{
    return m_localFilename;
}

void
CachedFile::checkFile()
{
    //!!! n.b. obvious race condition here if different CachedFile
    // objects for same url used in more than one thread -- need to
    // lock appropriately.  also consider race condition between
    // separate instances of the program!

    OriginLocalFilenameMap::const_iterator i = m_knownGoodCaches.find(m_origin);
    if (i != m_knownGoodCaches.end()) {
        m_ok = true;
        m_localFilename = i->second;
        return;
    }

    m_localFilename = getLocalFilenameFor(m_origin);

    if (!QFileInfo(m_localFilename).exists()) {
        cerr << "CachedFile::check: Local file does not exist, making a note that it hasn't been retrieved" << endl;
        updateLastRetrieval(false); // empirically!
    }

    QDateTime lastRetrieval = getLastRetrieval();

    if (lastRetrieval.isValid()) {
        cerr << "CachedFile::check: Valid last retrieval at "
                  << lastRetrieval.toString() << endl;
        // this will not be the case if the file is missing, after
        // updateLastRetrieval(false) was called above
        m_ok = true;
        if (lastRetrieval.addDays(2) < QDateTime::currentDateTime()) { //!!!
            cerr << "CachedFile::check: Out of date; trying to retrieve again" << endl;
            // doesn't matter if retrieval fails -- we just don't
            // update the last retrieval time

            //!!! but we do want an additional last-attempted
            // timestamp so as to ensure we aren't retrying the
            // retrieval every single time if it isn't working

            if (retrieve()) {
                cerr << "CachedFile::check: Retrieval succeeded" << endl;
                updateLastRetrieval(true);
            } else {
                cerr << "CachedFile::check: Retrieval failed, will try again later (using existing file for now)" << endl;
            }                
        }
    } else {
        cerr << "CachedFile::check: No valid last retrieval" << endl;
        // there is no acceptable file
        if (retrieve()) {
            cerr << "CachedFile::check: Retrieval succeeded" << endl;
            m_ok = true;
            updateLastRetrieval(true);
        } else {
            cerr << "CachedFile::check: Retrieval failed, remaining in invalid state" << endl;
            // again, we don't need to do anything here -- the last
            // retrieval timestamp is already invalid
        }
    }

    if (m_ok) {
        m_knownGoodCaches[m_origin] = m_localFilename;
    }
}

bool
CachedFile::retrieve()
{
    //!!! need to work by retrieving the file to another name, and
    //!!! then "atomically" moving it to its proper place (I'm not
    //!!! sure we can do an atomic move to replace an existing file
    //!!! using Qt classes, but a plain delete then copy is probably
    //!!! good enough)

    FileSource fs(m_origin, m_reporter, m_preferredContentType);

    if (!fs.isOK() || !fs.isAvailable()) {
        cerr << "CachedFile::retrieve: ERROR: FileSource reported unavailable or failure" << endl;
        return false;
    }

    fs.waitForData();

    if (!fs.isOK()) {
        cerr << "CachedFile::retrieve: ERROR: FileSource reported failure during receive" << endl;
        return false;
    }

    QString tempName = fs.getLocalFilename();
    QFile tempFile(tempName);
    if (!tempFile.exists()) {
        cerr << "CachedFile::retrieve: ERROR: FileSource reported success, but local temporary file \"" << tempName << "\" does not exist" << endl;
        return false;
    }

    QFile previous(m_localFilename);
    if (previous.exists()) {
        if (!previous.remove()) {
            cerr << "CachedFile::retrieve: ERROR: Failed to remove previous copy of cached file at \"" << m_localFilename << "\"" << endl;
            return false;
        }
    }

    //!!! This is not ideal, could leave us with nothing (old file
    //!!! removed, new file not able to be copied in because e.g. no
    //!!! disk space left)

    if (!tempFile.copy(m_localFilename)) {
        cerr << "CachedFile::retrieve: ERROR: Failed to copy newly retrieved file from \"" << tempName << "\" to \"" << m_localFilename << "\"" << endl;
        return false;
    }

    cerr << "CachedFile::retrieve: Successfully copied newly retrieved file \"" << tempName << "\" to its home at \"" << m_localFilename << "\"" << endl;

    return true;
}

QDateTime
CachedFile::getLastRetrieval()
{
    QSettings settings;
    settings.beginGroup("FileCache");

    QString key("last-retrieval-times");

    QMap<QString, QVariant> timeMap = settings.value(key).toMap();
    QDateTime lastTime = timeMap[m_localFilename].toDateTime();

    settings.endGroup();
    return lastTime;
}

void
CachedFile::updateLastRetrieval(bool successful)
{
    //!!! note !successful does not mean "we failed to update the
    //!!! file" (and so it remains the same as before); it means "the
    //!!! file is not there at all"
    
    QSettings settings;
    settings.beginGroup("FileCache");

    QString key("last-retrieval-times");

    QMap<QString, QVariant> timeMap = settings.value(key).toMap();

    QDateTime dt;
    if (successful) dt = QDateTime::currentDateTime();

    timeMap[m_localFilename] = dt;
    settings.setValue(key, timeMap);

    settings.endGroup();
}


