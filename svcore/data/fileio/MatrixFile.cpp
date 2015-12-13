/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2009 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "MatrixFile.h"
#include "base/TempDirectory.h"
#include "system/System.h"
#include "base/Profiler.h"
#include "base/Exceptions.h"
#include "base/Thread.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

#include <cstdio>
#include <cassert>

#include <cstdlib>

#include <QFileInfo>
#include <QDir>

//#define DEBUG_MATRIX_FILE 1
//#define DEBUG_MATRIX_FILE_READ_SET 1

#ifdef DEBUG_MATRIX_FILE_READ_SET
#ifndef DEBUG_MATRIX_FILE
#define DEBUG_MATRIX_FILE 1
#endif
#endif

std::map<QString, int> MatrixFile::m_refcount;
QMutex MatrixFile::m_createMutex;

static size_t totalStorage = 0;
static size_t totalCount = 0;
static size_t openCount = 0;

MatrixFile::MatrixFile(QString fileBase, Mode mode,
                       int cellSize, int width, int height) :
    m_fd(-1),
    m_mode(mode),
    m_flags(0),
    m_fmode(0),
    m_cellSize(cellSize),
    m_width(width),
    m_height(height),
    m_headerSize(2 * sizeof(int)),
    m_setColumns(0),
    m_autoClose(false),
    m_readyToReadColumn(-1)
{
    Profiler profiler("MatrixFile::MatrixFile", true);

#ifdef DEBUG_MATRIX_FILE
    cerr << "MatrixFile::MatrixFile(" << fileBase << ", " << int(mode) << ", " << cellSize << ", " << width << ", " << height << ")" << endl;
#endif

    m_createMutex.lock();

    QDir tempDir(TempDirectory::getInstance()->getPath());
    QString fileName(tempDir.filePath(QString("%1.mfc").arg(fileBase)));
    bool newFile = !QFileInfo(fileName).exists();

    if (newFile && m_mode == ReadOnly) {
        cerr << "ERROR: MatrixFile::MatrixFile: Read-only mode "
                  << "specified, but cache file does not exist" << endl;
        throw FileNotFound(fileName);
    }

    if (!newFile && m_mode == WriteOnly) {
        cerr << "ERROR: MatrixFile::MatrixFile: Write-only mode "
                  << "specified, but file already exists" << endl;
        throw FileOperationFailed(fileName, "create");
    }

    m_flags = 0;
    m_fmode = S_IRUSR | S_IWUSR;

    if (m_mode == WriteOnly) {
        m_flags = O_WRONLY | O_CREAT;
    } else {
        m_flags = O_RDONLY;
    }

#ifdef _WIN32
    m_flags |= O_BINARY;
#endif

#ifdef DEBUG_MATRIX_FILE
    cerr << "MatrixFile(" << this << ")::MatrixFile: opening " << fileName << "..." << endl;
#endif

    if ((m_fd = ::open(fileName.toLocal8Bit(), m_flags, m_fmode)) < 0) {
        ::perror("Open failed");
        cerr << "ERROR: MatrixFile::MatrixFile: "
                  << "Failed to open cache file \""
                  << fileName << "\"";
        if (m_mode == WriteOnly) cerr << " for writing";
        cerr << endl;
        throw FailedToOpenFile(fileName);
    }

    m_createMutex.unlock();

#ifdef DEBUG_MATRIX_FILE
    cerr << "MatrixFile(" << this << ")::MatrixFile: fd is " << m_fd << endl;
#endif

    if (newFile) {
        initialise(); // write header and "unwritten" column tags
    } else {
        int header[2];
        if (::read(m_fd, header, 2 * sizeof(int)) < 0) {
            ::perror("MatrixFile::MatrixFile: read failed");
            cerr << "ERROR: MatrixFile::MatrixFile: "
                      << "Failed to read header (fd " << m_fd << ", file \""
                      << fileName << "\")" << endl;
            throw FileReadFailed(fileName);
        }
        if (header[0] != m_width || header[1] != m_height) {
            cerr << "ERROR: MatrixFile::MatrixFile: "
                      << "Dimensions in file header (" << header[0] << "x"
                      << header[1] << ") differ from expected dimensions "
                      << m_width << "x" << m_height << endl;
            throw FailedToOpenFile(fileName);
        }
    }

    m_fileName = fileName;
    ++m_refcount[fileName];

#ifdef DEBUG_MATRIX_FILE
    cerr << "MatrixFile[" << m_fd << "]::MatrixFile: File " << fileName << ", ref " << m_refcount[fileName] << endl;

    cerr << "MatrixFile[" << m_fd << "]::MatrixFile: Done, size is " << "(" << m_width << ", " << m_height << ")" << endl;
#endif

    ++totalCount;
    ++openCount;
}

MatrixFile::~MatrixFile()
{
    if (m_fd >= 0) {
        if (::close(m_fd) < 0) {
            ::perror("MatrixFile::~MatrixFile: close failed");
        }
        openCount --;
    }

    QMutexLocker locker(&m_createMutex);

    delete m_setColumns;

    if (m_fileName != "") {

        if (--m_refcount[m_fileName] == 0) {

            if (::unlink(m_fileName.toLocal8Bit())) {
                cerr << "WARNING: MatrixFile::~MatrixFile: reference count reached 0, but failed to unlink file \"" << m_fileName << "\"" << endl;
            } else {
                cerr << "deleted " << m_fileName << endl;
            }
        }
    }

    if (m_mode == WriteOnly) {
        totalStorage -= (m_headerSize + (m_width * m_height * m_cellSize) + m_width);
    }
    totalCount --;

#ifdef DEBUG_MATRIX_FILE
    cerr << "MatrixFile[" << m_fd << "]::~MatrixFile: " << endl;
    cerr << "MatrixFile: Total storage now " << totalStorage/1024 << "K in " << totalCount << " instances (" << openCount << " open)" << endl;
#endif
}

void
MatrixFile::initialise()
{
    Profiler profiler("MatrixFile::initialise", true);

    assert(m_mode == WriteOnly);

    m_setColumns = new ResizeableBitset(m_width);
    
    off_t off = m_headerSize + (m_width * m_height * m_cellSize) + m_width;

#ifdef DEBUG_MATRIX_FILE
    cerr << "MatrixFile[" << m_fd << "]::initialise(" << m_width << ", " << m_height << "): cell size " << m_cellSize << ", header size " << m_headerSize << ", resizing file" << endl;
#endif

    if (::lseek(m_fd, off - 1, SEEK_SET) < 0) {
        ::perror("ERROR: MatrixFile::initialise: seek to end failed");
        throw FileOperationFailed(m_fileName, "lseek");
    }

    unsigned char byte = 0;
    if (::write(m_fd, &byte, 1) != 1) {
        ::perror("ERROR: MatrixFile::initialise: write at end failed");
        throw FileOperationFailed(m_fileName, "write");
    }

    if (::lseek(m_fd, 0, SEEK_SET) < 0) {
        ::perror("ERROR: MatrixFile::initialise: Seek to write header failed");
        throw FileOperationFailed(m_fileName, "lseek");
    }

    int header[2];
    header[0] = m_width;
    header[1] = m_height;
    if (::write(m_fd, header, 2 * sizeof(int)) != 2 * sizeof(int)) {
        ::perror("ERROR: MatrixFile::initialise: Failed to write header");
        throw FileOperationFailed(m_fileName, "write");
    }

    if (m_mode == WriteOnly) {
        totalStorage += (m_headerSize + (m_width * m_height * m_cellSize) + m_width);
    }

#ifdef DEBUG_MATRIX_FILE
    cerr << "MatrixFile[" << m_fd << "]::initialise(" << m_width << ", " << m_height << "): storage "
              << (m_headerSize + m_width * m_height * m_cellSize + m_width) << endl;

    cerr << "MatrixFile: Total storage " << totalStorage/1024 << "K" << endl;
#endif

    seekTo(0);
}

void
MatrixFile::close()
{
#ifdef DEBUG_MATRIX_FILE
    cerr << "MatrixFile::close()" << endl;
#endif
    if (m_fd >= 0) {
        if (::close(m_fd) < 0) {
            ::perror("MatrixFile::close: close failed");
        }
        m_fd = -1;
        -- openCount;
#ifdef DEBUG_MATRIX_FILE
        cerr << "MatrixFile: Now " << openCount << " open instances" << endl;
#endif
    }
}

void
MatrixFile::getColumnAt(int x, void *data)
{
    assert(m_mode == ReadOnly);
    
#ifdef DEBUG_MATRIX_FILE_READ_SET
    cerr << "MatrixFile[" << m_fd << "]::getColumnAt(" << x << ")" << endl;
#endif

    Profiler profiler("MatrixFile::getColumnAt");

    ssize_t r = -1;

    if (m_readyToReadColumn < 0 ||
        m_readyToReadColumn != x) {

        unsigned char set = 0;
        if (!seekTo(x)) {
            cerr << "ERROR: MatrixFile::getColumnAt(" << x << "): Seek failed" << endl;
            throw FileOperationFailed(m_fileName, "seek");
        }

        r = ::read(m_fd, &set, 1);
        if (r < 0) {
            ::perror("MatrixFile::getColumnAt: read failed");
            throw FileReadFailed(m_fileName);
        }
        if (!set) {
            cerr << "MatrixFile[" << m_fd << "]::getColumnAt(" << x << "): Column has not been set" << endl;
            return;
        }
    }

    r = ::read(m_fd, data, m_height * m_cellSize);
    if (r < 0) {
        ::perror("MatrixFile::getColumnAt: read failed");
        throw FileReadFailed(m_fileName);
    }
}

bool
MatrixFile::haveSetColumnAt(int x) const
{
    if (m_mode == WriteOnly) {
        return m_setColumns->get(x);
    }

    if (m_readyToReadColumn >= 0 &&
        int(m_readyToReadColumn) == x) return true;
    
    Profiler profiler("MatrixFile::haveSetColumnAt");

#ifdef DEBUG_MATRIX_FILE_READ_SET
    cerr << "MatrixFile[" << m_fd << "]::haveSetColumnAt(" << x << ")" << endl;
//    cerr << ".";
#endif

    unsigned char set = 0;
    if (!seekTo(x)) {
        cerr << "ERROR: MatrixFile::haveSetColumnAt(" << x << "): Seek failed" << endl;
        throw FileOperationFailed(m_fileName, "seek");
    }

    ssize_t r = -1;
    r = ::read(m_fd, &set, 1);
    if (r < 0) {
        ::perror("MatrixFile::haveSetColumnAt: read failed");
        throw FileReadFailed(m_fileName);
    }

    if (set) m_readyToReadColumn = int(x);

    return set;
}

void
MatrixFile::setColumnAt(int x, const void *data)
{
    assert(m_mode == WriteOnly);
    if (m_fd < 0) return; // closed

#ifdef DEBUG_MATRIX_FILE_READ_SET
    cerr << "MatrixFile[" << m_fd << "]::setColumnAt(" << x << ")" << endl;
//    cerr << ".";
#endif

    ssize_t w = 0;

    if (!seekTo(x)) {
        cerr << "ERROR: MatrixFile::setColumnAt(" << x << "): Seek failed" << endl;
        throw FileOperationFailed(m_fileName, "seek");
    }

    unsigned char set = 0;
    w = ::write(m_fd, &set, 1);
    if (w != 1) {
        ::perror("WARNING: MatrixFile::setColumnAt: write failed (1)");
        throw FileOperationFailed(m_fileName, "write");
    }

    w = ::write(m_fd, data, m_height * m_cellSize);
    if (w != ssize_t(m_height * m_cellSize)) {
        ::perror("WARNING: MatrixFile::setColumnAt: write failed (2)");
        throw FileOperationFailed(m_fileName, "write");
    }
/*
    if (x == 0) {
        cerr << "Wrote " << m_height * m_cellSize << " bytes, as follows:" << endl;
        for (int i = 0; i < m_height * m_cellSize; ++i) {
            cerr << (int)(((char *)data)[i]) << " ";
        }
        cerr << endl;
    }
*/
    if (!seekTo(x)) {
        cerr << "MatrixFile[" << m_fd << "]::setColumnAt(" << x << "): Seek failed" << endl;
        throw FileOperationFailed(m_fileName, "seek");
    }

    set = 1;
    w = ::write(m_fd, &set, 1);
    if (w != 1) {
        ::perror("WARNING: MatrixFile::setColumnAt: write failed (3)");
        throw FileOperationFailed(m_fileName, "write");
    }

    m_setColumns->set(x);
    if (m_autoClose) {
        if (m_setColumns->isAllOn()) {
#ifdef DEBUG_MATRIX_FILE
            cerr << "MatrixFile[" << m_fd << "]::setColumnAt(" << x << "): All columns set: auto-closing" << endl;
#endif
            close();
/*
        } else {
            int set = 0;
            for (int i = 0; i < m_width; ++i) {
                if (m_setColumns->get(i)) ++set;
            }
            cerr << "MatrixFile[" << m_fd << "]::setColumnAt(" << x << "): Auto-close on, but not all columns set yet (" << set << " of " << m_width << ")" << endl;
*/
        }
    }
}

bool
MatrixFile::seekTo(int x) const
{
    if (m_fd < 0) {
        cerr << "ERROR: MatrixFile::seekTo: File not open" << endl;
        return false;
    }

    m_readyToReadColumn = -1; // not ready, unless this is subsequently re-set

    off_t off = m_headerSize + x * m_height * m_cellSize + x;

#ifdef DEBUG_MATRIX_FILE_READ_SET
    if (m_mode == ReadOnly) {
        cerr << "MatrixFile[" << m_fd << "]::seekTo(" << x << "): off = " << off << endl;
    }
#endif

#ifdef DEBUG_MATRIX_FILE_READ_SET
    cerr << "MatrixFile[" << m_fd << "]::seekTo(" << x << "): off = " << off << endl;
#endif

    if (::lseek(m_fd, off, SEEK_SET) == (off_t)-1) {
        ::perror("Seek failed");
        cerr << "ERROR: MatrixFile::seekTo(" << x 
                  << ") = " << off << " failed" << endl;
        return false;
    }

    return true;
}

