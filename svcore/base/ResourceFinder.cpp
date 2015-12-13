/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

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
   This file copyright 2005-2011 Chris Cannam and the Rosegarden
   development team.
*/

#include "ResourceFinder.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QProcess>
#include <QCoreApplication>

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#endif

#include <cstdlib>
#include <iostream>

/**
   Resource files may be found in three places:

   * Bundled into the application as Qt4 resources.  These may be
     opened using Qt classes such as QFile, with "fake" file paths
     starting with a colon.  For example ":icons/fileopen.png".

   * Installed with the package, or in the user's equivalent home
     directory location.  For example,

     - on Linux, in /usr/share/<appname> or /usr/local/share/<appname>
     - on Linux, in $HOME/.local/share/<appname>

     - on OS/X, in /Library/Application Support/<appname>
     - on OS/X, in $HOME/Library/Application Support/<appname>

     - on Windows, in %ProgramFiles%/<company>/<appname>
     - on Windows, in (where?) something from http://msdn.microsoft.com/en-us/library/dd378457%28v=vs.85%29.aspx ?

   These locations are searched in reverse order (user-installed
   copies take priority over system-installed copies take priority
   over bundled copies).  Also, /usr/local takes priority over /usr.
*/

QStringList
ResourceFinder::getSystemResourcePrefixList()
{
    // returned in order of priority

    QStringList list;

#ifdef Q_OS_WIN32
    char *programFiles = getenv("ProgramFiles");
    if (programFiles && programFiles[0]) {
        list << QString("%1/%2/%3")
            .arg(programFiles)
            .arg(qApp->organizationName())
            .arg(qApp->applicationName());
    } else {
        list << QString("C:/Program Files/%1/%2")
            .arg(qApp->organizationName())
            .arg(qApp->applicationName());
    }
#else
#ifdef Q_OS_MAC
    list << QString("/Library/Application Support/%1")
        .arg(qApp->applicationName());
#else
    list << QString("/usr/local/share/%1")
        .arg(qApp->applicationName());
    list << QString("/usr/share/%1")
        .arg(qApp->applicationName());
#endif
#endif    

    return list;
}

static QString
getOldStyleUserResourcePrefix()
{
#ifdef Q_OS_WIN32
    // This is awkward and does not work correctly for non-ASCII home
    // directory names, hence getNewStyleUserResourcePrefix() below
    char *homedrive = getenv("HOMEDRIVE");
    char *homepath = getenv("HOMEPATH");
    QString home;
    if (homedrive && homepath) {
        home = QString("%1%2").arg(homedrive).arg(homepath);
    } else {
        home = QDir::home().absolutePath();
    }
    if (home == "") return "";
    return QString("%1/.%2").arg(home).arg(qApp->applicationName()); //!!! wrong
#else
    char *home = getenv("HOME");
    if (!home || !home[0]) return "";
#ifdef Q_OS_MAC
    return QString("%1/Library/Application Support/%2")
        .arg(home)
        .arg(qApp->applicationName());
#else
    return QString("%1/.local/share/%2")
        .arg(home)
        .arg(qApp->applicationName());
#endif
#endif
}

static QString
getNewStyleUserResourcePrefix()
{
#if QT_VERSION >= 0x050000
    // This is expected to be much more reliable than
    // getOldStyleUserResourcePrefix(), but it returns a different
    // directory because it includes the organisation name (which is
    // fair enough). Hence migrateOldStyleResources() which moves
    // across any resources found in the old-style path the first time
    // we look for the new-style one
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    return getOldStyleUserResourcePrefix();
#endif
}

static void
migrateOldStyleResources()
{
    QString oldPath = getOldStyleUserResourcePrefix();
    QString newPath = getNewStyleUserResourcePrefix();
    
    if (oldPath != newPath &&
        QDir(oldPath).exists() &&
        !QDir(newPath).exists()) {

        QDir d(oldPath);
        
        if (!d.mkpath(newPath)) {
            cerr << "WARNING: Failed to create new-style resource path \""
                 << newPath << "\" to migrate old resources to" << endl;
            return;
        }

        QDir target(newPath);

        bool success = true;

        QStringList entries
            (d.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot));

        foreach (QString entry, entries) {
            if (d.rename(entry, target.filePath(entry))) {
                cerr << "NOTE: Successfully moved resource \""
                     << entry << "\" from old resource path to new" << endl;
            } else {
                cerr << "WARNING: Failed to move old resource \""
                     << entry << "\" from old location \""
                     << oldPath << "\" to new location \""
                     << newPath << "\"" << endl;
                success = false;
            }
        }

        if (success) {
            if (!d.rmdir(oldPath)) {
                cerr << "WARNING: Failed to remove old resource path \""
                     << oldPath << "\" after migrating " << entries.size()
                     << " resource(s) to new path \"" << newPath
                     << "\" (directory not empty?)" << endl;
            } else {
                cerr << "NOTE: Successfully moved " << entries.size()
                     << " resource(s) from old resource "
                     << "path \"" << oldPath << "\" to new path \""
                     << newPath << "\"" << endl;
            }
        }
    }
}

QString
ResourceFinder::getUserResourcePrefix()
{
    migrateOldStyleResources();
    return getNewStyleUserResourcePrefix();
}

QStringList
ResourceFinder::getResourcePrefixList()
{
    // returned in order of priority

    QStringList list;

    QString user = getUserResourcePrefix();
    if (user != "") list << user;

    list << getSystemResourcePrefixList();

    list << ":"; // bundled resource location

    return list;
}

QString
ResourceFinder::getResourcePath(QString resourceCat, QString fileName)
{
    // We don't simply call getResourceDir here, because that returns
    // only the "installed file" location.  We also want to search the
    // bundled resources and user-saved files.

    QStringList prefixes = getResourcePrefixList();
    
    if (resourceCat != "") resourceCat = "/" + resourceCat;

    for (QStringList::const_iterator i = prefixes.begin();
         i != prefixes.end(); ++i) {
        
        QString prefix = *i;

        cerr << "ResourceFinder::getResourcePath: Looking up file \"" << fileName << "\" for category \"" << resourceCat << "\" in prefix \"" << prefix << "\"" << endl;

        QString path =
            QString("%1%2/%3").arg(prefix).arg(resourceCat).arg(fileName);
        if (QFileInfo(path).exists() && QFileInfo(path).isReadable()) {
            cerr << "Found it!" << endl;
            return path;
        }
    }

    return "";
}

QString
ResourceFinder::getResourceDir(QString resourceCat)
{
    // Returns only the "installed file" location

    QStringList prefixes = getSystemResourcePrefixList();
    
    if (resourceCat != "") resourceCat = "/" + resourceCat;

    for (QStringList::const_iterator i = prefixes.begin();
         i != prefixes.end(); ++i) {
        
        QString prefix = *i;
        QString path = QString("%1%2").arg(prefix).arg(resourceCat);
        if (QFileInfo(path).exists() &&
            QFileInfo(path).isDir() &&
            QFileInfo(path).isReadable()) {
            return path;
        }
    }

    return "";
}

QString
ResourceFinder::getResourceSavePath(QString resourceCat, QString fileName)
{
    QString dir = getResourceSaveDir(resourceCat);
    if (dir == "") return "";

    return dir + "/" + fileName;
}

QString
ResourceFinder::getResourceSaveDir(QString resourceCat)
{
    // Returns the "user" location

    QString user = getUserResourcePrefix();
    if (user == "") return "";

    if (resourceCat != "") resourceCat = "/" + resourceCat;

    QDir userDir(user);
    if (!userDir.exists()) {
        if (!userDir.mkpath(user)) {
            cerr << "ResourceFinder::getResourceSaveDir: ERROR: Failed to create user resource path \"" << user << "\"" << endl;
            return "";
        }
    }

    if (resourceCat != "") {
        QString save = QString("%1%2").arg(user).arg(resourceCat);
        QDir saveDir(save);
        if (!saveDir.exists()) {
            if (!saveDir.mkpath(save)) {
                cerr << "ResourceFinder::getResourceSaveDir: ERROR: Failed to create user resource path \"" << save << "\"" << endl;
                return "";
            }
        }
        return save;
    } else {
        return user;
    }
}

QStringList
ResourceFinder::getResourceFiles(QString resourceCat, QString fileExt)
{
    QStringList results;
    QStringList prefixes = getResourcePrefixList();

    QStringList filters;
    filters << QString("*.%1").arg(fileExt);

    for (QStringList::const_iterator i = prefixes.begin();
         i != prefixes.end(); ++i) {
        
        QString prefix = *i;
        QString path;

        if (resourceCat != "") {
            path = QString("%1/%2").arg(prefix).arg(resourceCat);
        } else {
            path = prefix;
        }
        
        QDir dir(path);
        if (!dir.exists()) continue;

        dir.setNameFilters(filters);
        QStringList entries = dir.entryList
            (QDir::Files | QDir::Readable, QDir::Name);
        
        for (QStringList::const_iterator j = entries.begin();
             j != entries.end(); ++j) {
            results << QString("%1/%2").arg(path).arg(*j);
        }
    }

    return results;
}

bool
ResourceFinder::unbundleResource(QString resourceCat, QString fileName)
{
    QString path = getResourcePath(resourceCat, fileName);
    
    if (!path.startsWith(':')) return true;

    // This is the lowest-priority alternative path for this
    // resource, so we know that there must be no installed copy.
    // Install one to the user location.
    cerr << "ResourceFinder::unbundleResource: File " << fileName << " is bundled, un-bundling it" << endl;
    QString target = getResourceSavePath(resourceCat, fileName);
    QFile file(path);
    if (!file.copy(target)) {
        cerr << "ResourceFinder::unbundleResource: ERROR: Failed to un-bundle resource file \"" << fileName << "\" to user location \"" << target << "\"" << endl;
        return false;
    }

    QFile chmod(target);
    chmod.setPermissions(QFile::ReadOwner |
                         QFile::ReadUser  | /* for potential platform-independence */
                         QFile::ReadGroup |
                         QFile::ReadOther |
                         QFile::WriteOwner|
                         QFile::WriteUser); /* for potential platform-independence */

    return true;
}

