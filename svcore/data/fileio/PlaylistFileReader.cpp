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

#include "PlaylistFileReader.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QFileInfo>
#include <QDir>

#include <iostream>

PlaylistFileReader::PlaylistFileReader(QString path) :
    m_source(path),
    m_file(0)
{
    if (!m_source.isAvailable()) {
        m_error = QFile::tr("File or URL \"%1\" could not be retrieved")
            .arg(path);
        return;
    }
    init();
}

PlaylistFileReader::PlaylistFileReader(FileSource source) :
    m_source(source),
    m_file(0)
{
    if (!m_source.isAvailable()) {
        m_error = QFile::tr("File or URL \"%1\" could not be retrieved")
            .arg(source.getLocation());
        return;
    }
    init();
}

PlaylistFileReader::~PlaylistFileReader()
{
    if (m_file) m_file->close();
    delete m_file;
}

void
PlaylistFileReader::init()
{
    if (!m_source.isAvailable()) return;

    m_source.waitForData();

    QString filename = m_source.getLocalFilename();

    m_file = new QFile(filename);
    bool good = false;

    if (!m_file->exists()) {
	m_error = QFile::tr("File \"%1\" does not exist")
            .arg(m_source.getLocation());
    } else if (!m_file->open(QIODevice::ReadOnly | QIODevice::Text)) {
	m_error = QFile::tr("Failed to open file \"%1\"")
            .arg(m_source.getLocation());
    } else {
	good = true;
    }

    if (good) {
        if (!m_source.isRemote()) {
            m_basedir = QFileInfo(filename).dir().canonicalPath();
        }
    }

    if (!good) {
	delete m_file;
	m_file = 0;
    }
}

bool
PlaylistFileReader::isOK() const
{
    return (m_file != 0);
}

QString
PlaylistFileReader::getError() const
{
    return m_error;
}

PlaylistFileReader::Playlist
PlaylistFileReader::load() const
{
    if (!m_file) return Playlist();

    QTextStream in(m_file);
    in.seek(0);

    Playlist playlist;

    while (!in.atEnd()) {

        // cope with old-style Mac line endings (c.f. CSVFileReader)
        // as well as DOS/Unix style

        QString chunk = in.readLine();
        QStringList lines = chunk.split('\r', QString::SkipEmptyParts);
        
        for (int li = 0; li < lines.size(); ++li) {

            QString line = lines[li];

            if (line.startsWith("#")) continue;

            // line is expected to be a URL or a file path.  If it
            // appears to be a local relative file path, then we
            // should check whether it can be resolved relative to the
            // location of the playlist file and, if so, do so.

            if (!FileSource::isRemote(line)) {
                if (QFileInfo(line).isRelative() && m_basedir != "") {
                    QString testpath = QDir(m_basedir).filePath(line);
                    if (QFileInfo(testpath).exists() &&
                        QFileInfo(testpath).isFile()) {
                        cerr << "Path \"" << line
                                  << "\" is relative, resolving to \""
                                  << testpath << "\""
                                  << endl;
                        line = testpath;
                    }
                }
            }

            playlist.push_back(line);
        }
    }

    return playlist;
}

void
PlaylistFileReader::getSupportedExtensions(std::set<QString> &extensions)
{
    extensions.insert("m3u");
}
