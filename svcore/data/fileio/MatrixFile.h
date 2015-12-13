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

#ifndef _MATRIX_FILE_CACHE_H_
#define _MATRIX_FILE_CACHE_H_

#include "base/ResizeableBitset.h"

#include "FileReadThread.h"

#include <sys/types.h>
#include <QString>
#include <QMutex>
#include <map>

class MatrixFile : public QObject
{
    Q_OBJECT

public:
    enum Mode { ReadOnly, WriteOnly };

    /**
     * Construct a MatrixFile object reading from and/or writing to
     * the matrix file with the given base name in the application's
     * temporary directory.
     *
     * If mode is ReadOnly, the file must exist and be readable.
     *
     * If mode is WriteOnly, the file must not exist.
     *
     * cellSize specifies the size in bytes of the object type stored
     * in the matrix.  For example, use cellSize = sizeof(float) for a
     * matrix of floats.  The MatrixFile object doesn't care about the
     * objects themselves, it just deals with raw data of a given size.
     *
     * width and height specify the dimensions of the file.  These
     * cannot be changed after construction.
     *
     * MatrixFiles are reference counted by name.  When the last
     * MatrixFile with a given name is destroyed, the file is removed.
     * These are temporary files; the normal usage is to have one
     * MatrixFile of WriteOnly type creating the file and then
     * persisting until all readers are complete.
     *
     * MatrixFile has no built-in cache and is not thread-safe.  Use a
     * separate MatrixFile in each thread.
     */
    MatrixFile(QString fileBase, Mode mode, int cellSize,
               int width, int height);
    virtual ~MatrixFile();

    Mode getMode() const { return m_mode; }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getCellSize() const { return m_cellSize; }

    /**
     * If this is set true on a write-mode MatrixFile, then the file
     * will close() itself when all columns have been written.
     */
    void setAutoClose(bool a) { m_autoClose = a; }

    void close(); // does not decrement ref count; that happens in dtor

    bool haveSetColumnAt(int x) const;
    void getColumnAt(int x, void *data); // may throw FileReadFailed
    void setColumnAt(int x, const void *data);

protected:
    int     m_fd;
    Mode    m_mode;
    int     m_flags;
    mode_t  m_fmode;
    int     m_cellSize;
    int     m_width;
    int     m_height;
    int     m_headerSize;
    QString m_fileName;

    ResizeableBitset *m_setColumns; // only in writer
    bool m_autoClose;

    // In reader: if this is >= 0, we can read that column directly
    // without seeking (and we know that the column exists)
    mutable int m_readyToReadColumn;

    static std::map<QString, int> m_refcount;
    static QMutex m_createMutex;

    void initialise();
    bool seekTo(int col) const;
};

#endif

