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

#include "MatchFileReader.h"

#include <QFile>
#include <QTextStream>

#include <cmath>
#include <iostream>

Alignment::Alignment() :
    thisHopTime(0.0),
    refHopTime(0.0)
{
}

double
Alignment::fromReference(double t) const
{
    int ri = lrint(t / refHopTime);
    int index = search(refIndex, ri);
    return thisIndex[index] * thisHopTime;
}

double
Alignment::toReference(double t) const
{
    int ti = lrint(t / thisHopTime);
    int index = search(thisIndex, ti);
    return refIndex[index] * refHopTime;
}

int
Alignment::search(const FrameArray &arr, int val) const
{
    int len = arr.size();
    int max = len - 1;
    int min = 0;
    while (max > min) {
        int mid = (max + min) / 2;
        if (val > arr[mid]) {
            min = mid + 1;
        } else {
            max = mid;
        }
    } // max = MIN_j (arr[j] >= val)   i.e. the first equal or next highest
    while ((max + 1 < len) && (arr[max + 1] == val)) {
        max++;
    }
    return (min + max) / 2;
}

MatchFileReader::MatchFileReader(QString path) :
    m_file(0)
{
    m_file = new QFile(path);
    bool good = false;
    
    if (!m_file->exists()) {
	m_error = QFile::tr("File \"%1\" does not exist").arg(path);
    } else if (!m_file->open(QIODevice::ReadOnly | QIODevice::Text)) {
	m_error = QFile::tr("Failed to open file \"%1\"").arg(path);
    } else {
	good = true;
    }

    if (!good) {
	delete m_file;
	m_file = 0;
    }
}

MatchFileReader::~MatchFileReader()
{
    if (m_file) {
        cerr << "MatchFileReader::MatchFileReader: Closing file" << endl;
        m_file->close();
    }
    delete m_file;
}

bool
MatchFileReader::isOK() const
{
    return (m_file != 0);
}

QString
MatchFileReader::getError() const
{
    return m_error;
}

Alignment
MatchFileReader::load() const
{
    Alignment alignment;

    if (!m_file) return alignment;

    QTextStream in(m_file);

/*
File: /home/studio/match-test/mahler-3-boulez-5.wav
Marks: -1
FixedPoints: true 0
0
0
0
0
File: /home/studio/match-test/mahler-3-haitink-5.wav
Marks: 0
FixedPoints: true 0
0.02
0.02
12836
*/

    int fileCount = 0;
    int state = 0;
    int count = 0;

    while (!in.atEnd()) {

        QString line = in.readLine().trimmed();
        if (line.startsWith("File: ")) {
            ++fileCount;
            continue;
        }
        if (fileCount != 2) continue;
        if (line.startsWith("Marks:") || line.startsWith("FixedPoints:")) {
            continue;
        }

        switch (state) {
        case 0:
            alignment.thisHopTime = line.toDouble();
            break;
        case 1:
            alignment.refHopTime = line.toDouble();
            break;
        case 2: 
            count = line.toInt();
            break;
        case 3:
            alignment.thisIndex.push_back(line.toInt());
            break;
        case 4:
            alignment.refIndex.push_back(line.toInt());
            break;
        }

        if (state < 3) ++state;
        else if (state == 3 && alignment.thisIndex.size() == count) ++state;
    }

    if (alignment.thisHopTime == 0.0) {
        cerr << "ERROR in Match file: this hop time == 0, using 0.01 instead" << endl;
        alignment.thisHopTime = 0.01;
    }

    if (alignment.refHopTime == 0.0) {
        cerr << "ERROR in Match file: ref hop time == 0, using 0.01 instead" << endl;
        alignment.refHopTime = 0.01;
    }

    cerr << "MatchFileReader: this hop = " << alignment.thisHopTime << ", ref hop = " << alignment.refHopTime << ", this index count = " << alignment.thisIndex.size() << ", ref index count = " << alignment.refIndex.size() << endl;

    return alignment;
}
