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

#ifndef _PLAYLIST_FILE_READER_H_
#define _PLAYLIST_FILE_READER_H_

#include "FileSource.h"

#include <QString>

#include <vector>
#include <set>

class QFile;

class PlaylistFileReader
{
public:
    typedef std::vector<QString> Playlist;

    PlaylistFileReader(QString path);
    PlaylistFileReader(FileSource source);
    virtual ~PlaylistFileReader();

    virtual bool isOK() const;
    virtual QString getError() const;
    virtual Playlist load() const;

    static void getSupportedExtensions(std::set<QString> &extensions);

protected:
    void init();

    FileSource m_source;
    QFile *m_file;
    QString m_basedir;
    QString m_error;
};

#endif
