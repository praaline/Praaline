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

#include "BZipFileDevice.h"

#include <bzlib.h>

#include <iostream>

#include "base/Debug.h"

BZipFileDevice::BZipFileDevice(QString fileName) :
    m_fileName(fileName),
    m_file(0),
    m_bzFile(0),
    m_atEnd(true),
    m_ok(true)
{
}

BZipFileDevice::~BZipFileDevice()
{
//    cerr << "BZipFileDevice::~BZipFileDevice(" << m_fileName << ")" << endl;
    if (m_bzFile) close();
}

bool
BZipFileDevice::isOK() const
{
    return m_ok;
}

bool
BZipFileDevice::open(OpenMode mode)
{
    setErrorString("");

    if (m_bzFile) {
        setErrorString(tr("File is already open"));
        return false;
    }

    if (mode & Append) {
        setErrorString(tr("Append mode not supported"));
        m_ok = false;
        return false;
    }

    if ((mode & (ReadOnly | WriteOnly)) == 0) {
        setErrorString(tr("File access mode not specified"));
        m_ok = false;
        return false;
    }

    if ((mode & ReadOnly) && (mode & WriteOnly)) {
        setErrorString(tr("Read and write modes both specified"));
        m_ok = false;
        return false;
    }

    if (mode & WriteOnly) {

        m_file = fopen(m_fileName.toLocal8Bit().data(), "wb");
        if (!m_file) {
            setErrorString(tr("Failed to open file for writing"));
            m_ok = false;
            return false;
        }

        int bzError = BZ_OK;
        m_bzFile = BZ2_bzWriteOpen(&bzError, m_file, 9, 0, 0);

        if (!m_bzFile) {
            fclose(m_file);
            m_file = 0;
            setErrorString(tr("Failed to open bzip2 stream for writing"));
            m_ok = false;
            return false;
        }

//        cerr << "BZipFileDevice: opened \"" << m_fileName << "\" for writing" << endl;

        setErrorString(QString());
        setOpenMode(mode);
        return true;
    }

    if (mode & ReadOnly) {

        m_file = fopen(m_fileName.toLocal8Bit().data(), "rb");
        if (!m_file) {
            setErrorString(tr("Failed to open file for reading"));
            m_ok = false;
            return false;
        }

        int bzError = BZ_OK;
        m_bzFile = BZ2_bzReadOpen(&bzError, m_file, 0, 0, NULL, 0);

        if (!m_bzFile) {
            fclose(m_file);
            m_file = 0;
            setErrorString(tr("Failed to open bzip2 stream for reading"));
            m_ok = false;
            return false;
        }

//        cerr << "BZipFileDevice: opened \"" << m_fileName << "\" for reading" << endl;

        m_atEnd = false;

        setErrorString(QString());
        setOpenMode(mode);
        return true;
    }

    setErrorString(tr("Internal error (open for neither read nor write)"));
    m_ok = false;
    return false;
}

void
BZipFileDevice::close()
{
    if (!m_bzFile) {
        setErrorString(tr("File not open"));
        m_ok = false;
        return;
    }

    int bzError = BZ_OK;

    if (openMode() & WriteOnly) {
        unsigned int in = 0, out = 0;
        BZ2_bzWriteClose(&bzError, m_bzFile, 0, &in, &out);
//	cerr << "Wrote bzip2 stream (in=" << in << ", out=" << out << ")" << endl;
	if (bzError != BZ_OK) {
	    setErrorString(tr("bzip2 stream write close error"));
	}
        fclose(m_file);
        m_bzFile = 0;
        m_file = 0;
        m_ok = false;
        return;
    }

    if (openMode() & ReadOnly) {
        BZ2_bzReadClose(&bzError, m_bzFile);
        if (bzError != BZ_OK) {
            setErrorString(tr("bzip2 stream read close error"));
        }
        fclose(m_file);
        m_bzFile = 0;
        m_file = 0;
        m_ok = false;
        return;
    }

    setErrorString(tr("Internal error (close for neither read nor write)"));
    return;
}

qint64
BZipFileDevice::readData(char *data, qint64 maxSize)
{
    if (m_atEnd) return 0;

    int bzError = BZ_OK;
    int read = BZ2_bzRead(&bzError, m_bzFile, data, int(maxSize));

//    cerr << "BZipFileDevice::readData: requested " << maxSize << ", read " << read << endl;

    if (bzError != BZ_OK) {
        if (bzError != BZ_STREAM_END) {
            cerr << "BZipFileDevice::readData: error condition" << endl;
            setErrorString(tr("bzip2 stream read error"));
            m_ok = false;
            return -1;
        } else {
//            cerr << "BZipFileDevice::readData: reached end of file" << endl;
            m_atEnd = true;
        }            
    }

    return read;
}

qint64
BZipFileDevice::writeData(const char *data, qint64 maxSize)
{
    int bzError = BZ_OK;
    BZ2_bzWrite(&bzError, m_bzFile, (void *)data, int(maxSize));

//    cerr << "BZipFileDevice::writeData: " << maxSize << " to write" << endl;

    if (bzError != BZ_OK) {
        cerr << "BZipFileDevice::writeData: error condition" << endl;
        setErrorString("bzip2 stream write error");
        m_ok = false;
        return -1;
    }

//    cerr << "BZipFileDevice::writeData: wrote " << maxSize << endl;

    return maxSize;
}

