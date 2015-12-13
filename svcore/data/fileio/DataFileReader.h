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

#ifndef _DATA_FILE_READER_H_
#define _DATA_FILE_READER_H_

#include <QString>
#include <QObject>

class Model;

class DataFileReader : public QObject
{
public:
    /**
     * Return true if the file appears to be of the correct type.
     *
     * The DataFileReader will be constructed by passing a file path
     * to its constructor.  If the file can at that time be determined
     * to be not of a type that this reader can read, it should return
     * false in response to any subsequent call to isOK().
     *
     * If the file is apparently of the correct type, isOK() should
     * return true; if it turns out that the file cannot after all be
     * read (because it's corrupted or the detection misfired), then
     * the read() function may return NULL.
     */
    virtual bool isOK() const = 0;

    virtual QString getError() const { return ""; }

    /**
     * Read the file and return the corresponding data model.  This
     * function is not expected to be thread-safe or reentrant.  This
     * function may be interactive (i.e. it's permitted to pop up
     * dialogs and windows and ask the user to specify any details
     * that can't be automatically extracted from the file).
     *
     * Return NULL if the file cannot be parsed at all (although it's
     * preferable to return a partial model and warn the user).
     *
     * Caller owns the returned model and must delete it after use.
     */
    virtual Model *load() const = 0;
};

#endif
