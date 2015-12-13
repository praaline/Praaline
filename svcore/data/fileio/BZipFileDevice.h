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

#ifndef _BZIP_FILE_DEVICE_H_
#define _BZIP_FILE_DEVICE_H_

#include <QIODevice>

#include <bzlib.h>

class BZipFileDevice : public QIODevice
{
    Q_OBJECT

public:
    BZipFileDevice(QString fileName);
    virtual ~BZipFileDevice();
    
    virtual bool open(OpenMode mode);
    virtual void close();

    virtual bool isOK() const;

    virtual bool isSequential() const { return true; }

protected:
    virtual qint64 readData(char *data, qint64 maxSize);
    virtual qint64 writeData(const char *data, qint64 maxSize);

    QString m_fileName;

    FILE *m_file;
    BZFILE *m_bzFile;
    bool m_atEnd;
    bool m_ok;
};

#endif
