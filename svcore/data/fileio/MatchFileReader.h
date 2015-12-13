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

#ifndef _MATCH_FILE_READER_H_
#define _MATCH_FILE_READER_H_

#include <vector>
#include <QString>
#include "base/Debug.h"

class QFile;
class Model;

class Alignment
{
public:
    Alignment();

    typedef std::vector<int> FrameArray;

    double thisHopTime;
    double refHopTime;

    FrameArray thisIndex;
    FrameArray refIndex;

    double fromReference(double) const;
    double toReference(double) const;

    //!!! blah
    void setMainModel(Model *m) { m_mainModel = m; }
    bool isMainModel(Model *m) const { return m == m_mainModel; }

    int search(const FrameArray &arr, int val) const;

protected:
    Model *m_mainModel;
};

class MatchFileReader
{
public:
    MatchFileReader(QString path);
    virtual ~MatchFileReader();

    virtual bool isOK() const;
    virtual QString getError() const;
    virtual Alignment load() const;

protected:
    QFile *m_file;
    QString m_error;
};

#endif

