/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "EditableDenseThreeDimensionalModel.h"

#include "base/LogRange.h"

#include <QTextStream>
#include <QStringList>
#include <QReadLocker>
#include <QWriteLocker>

#include <iostream>

#include <cmath>
#include <cassert>

using std::vector;

#include "system/System.h"

EditableDenseThreeDimensionalModel::EditableDenseThreeDimensionalModel(sv_samplerate_t sampleRate,
                                                                       int resolution,
                                                                       int yBinCount,
                                                                       CompressionType compression,
                                                                       bool notifyOnAdd) :
    m_startFrame(0),
    m_sampleRate(sampleRate),
    m_resolution(resolution),
    m_yBinCount(yBinCount),
    m_compression(compression),
    m_minimum(0.0),
    m_maximum(0.0),
    m_haveExtents(false),
    m_notifyOnAdd(notifyOnAdd),
    m_sinceLastNotifyMin(-1),
    m_sinceLastNotifyMax(-1),
    m_completion(100)
{
}    

bool
EditableDenseThreeDimensionalModel::isOK() const
{
    return true;
}

sv_samplerate_t
EditableDenseThreeDimensionalModel::getSampleRate() const
{
    return m_sampleRate;
}

sv_frame_t
EditableDenseThreeDimensionalModel::getStartFrame() const
{
    return m_startFrame;
}

void
EditableDenseThreeDimensionalModel::setStartFrame(sv_frame_t f)
{
    m_startFrame = f; 
}

sv_frame_t
EditableDenseThreeDimensionalModel::getEndFrame() const
{
    return m_resolution * m_data.size() + (m_resolution - 1);
}

int
EditableDenseThreeDimensionalModel::getResolution() const
{
    return m_resolution;
}

void
EditableDenseThreeDimensionalModel::setResolution(int sz)
{
    m_resolution = sz;
}

int
EditableDenseThreeDimensionalModel::getWidth() const
{
    return m_data.size();
}

int
EditableDenseThreeDimensionalModel::getHeight() const
{
    return m_yBinCount;
}

void
EditableDenseThreeDimensionalModel::setHeight(int sz)
{
    m_yBinCount = sz;
}

float
EditableDenseThreeDimensionalModel::getMinimumLevel() const
{
    return m_minimum;
}

void
EditableDenseThreeDimensionalModel::setMinimumLevel(float level)
{
    m_minimum = level;
}

float
EditableDenseThreeDimensionalModel::getMaximumLevel() const
{
    return m_maximum;
}

void
EditableDenseThreeDimensionalModel::setMaximumLevel(float level)
{
    m_maximum = level;
}

EditableDenseThreeDimensionalModel::Column
EditableDenseThreeDimensionalModel::getColumn(int index) const
{
    QReadLocker locker(&m_lock);
    if (index < 0 || index >= m_data.size()) return Column();
    return expandAndRetrieve(index);
}

float
EditableDenseThreeDimensionalModel::getValueAt(int index, int n) const
{
    Column c = getColumn(index);
    if (int(n) < c.size()) return c.at(n);
    return m_minimum;
}

//static int given = 0, stored = 0;

void
EditableDenseThreeDimensionalModel::truncateAndStore(int index,
                                                     const Column &values)
{
    assert(int(index) < m_data.size());

    //cout << "truncateAndStore(" << index << ", " << values.size() << ")" << endl;

    // The default case is to store the entire column at m_data[index]
    // and place 0 at m_trunc[index] to indicate that it has not been
    // truncated.  We only do clever stuff if one of the clever-stuff
    // tests works out.

    m_trunc[index] = 0;
    if (index == 0 ||
        m_compression == NoCompression ||
        values.size() != int(m_yBinCount)) {
//        given += values.size();
//        stored += values.size();
        m_data[index] = values;
        return;
    }

    // Maximum distance between a column and the one we refer to as
    // the source of its truncated values.  Limited by having to fit
    // in a signed char, but in any case small values are usually
    // better
    static int maxdist = 6;

    bool known = false; // do we know whether to truncate at top or bottom?
    bool top = false;   // if we do know, will we truncate at top?

    // If the previous column is not truncated, then it is the only
    // candidate for comparison.  If it is truncated, then the column
    // that it refers to is the only candidate.  Either way, we only
    // have one possible column to compare against here, and we are
    // being careful to ensure it is not a truncated one (to avoid
    // doing more work recursively when uncompressing).
    int tdist = 1;
    int ptrunc = m_trunc[index-1];
    if (ptrunc < 0) {
        top = false;
        known = true;
        tdist = -ptrunc + 1;
    } else if (ptrunc > 0) {
        top = true;
        known = true;
        tdist = ptrunc + 1;
    }

    Column p = expandAndRetrieve(index - tdist);
    int h = m_yBinCount;

    if (p.size() == h && tdist <= maxdist) {

        int bcount = 0, tcount = 0;
        if (!known || !top) {
            // count how many identical values there are at the bottom
            for (int i = 0; i < h; ++i) {
                if (values.at(i) == p.at(i)) ++bcount;
                else break;
            }
        }
        if (!known || top) {
            // count how many identical values there are at the top
            for (int i = h; i > 0; --i) {
                if (values.at(i-1) == p.at(i-1)) ++tcount;
                else break;
            }
        }
        if (!known) top = (tcount > bcount);

        int limit = h / 4; // don't bother unless we have at least this many
        if ((top ? tcount : bcount) > limit) {
        
            if (!top) {
                // create a new column with h - bcount values from bcount up
                Column tcol(h - bcount);
//                given += values.size();
//                stored += h - bcount;
                for (int i = bcount; i < h; ++i) {
                    tcol[i - bcount] = values.at(i);
                }
                m_data[index] = tcol;
                m_trunc[index] = (signed char)(-tdist);
                return;
            } else {
                // create a new column with h - tcount values from 0 up
                Column tcol(h - tcount);
//                given += values.size();
//                stored += h - tcount;
                for (int i = 0; i < h - tcount; ++i) {
                    tcol[i] = values.at(i);
                }
                m_data[index] = tcol;
                m_trunc[index] = (signed char)(tdist);
                return;
            }
        }
    }                

//    given += values.size();
//    stored += values.size();
//    cout << "given: " << given << ", stored: " << stored << " (" 
//              << ((float(stored) / float(given)) * 100.f) << "%)" << endl;

    // default case if nothing wacky worked out
    m_data[index] = values;
    return;
}

EditableDenseThreeDimensionalModel::Column
EditableDenseThreeDimensionalModel::expandAndRetrieve(int index) const
{
    // See comment above m_trunc declaration in header

    assert(index >= 0 && index < int(m_data.size()));
    Column c = m_data.at(index);
    if (index == 0) {
        return c;
    }
    int trunc = (int)m_trunc[index];
    if (trunc == 0) {
        return c;
    }
    bool top = true;
    int tdist = trunc;
    if (trunc < 0) { top = false; tdist = -trunc; }
    Column p = expandAndRetrieve(index - tdist);
    int psize = p.size(), csize = c.size();
    if (psize != m_yBinCount) {
        cerr << "WARNING: EditableDenseThreeDimensionalModel::expandAndRetrieve: Trying to expand from incorrectly sized column" << endl;
    }
    if (top) {
        for (int i = csize; i < psize; ++i) {
            c.push_back(p.at(i));
        }
    } else {
        // push_front is very slow on QVector -- but not enough to
        // make it desirable to choose a different container, since
        // QVector has all the other advantages for us.  easier to
        // write the whole array out to a new vector
        Column cc(psize);
        for (int i = 0; i < psize - csize; ++i) {
            cc[i] = p.at(i);
        }
        for (int i = 0; i < csize; ++i) {
            cc[i + (psize - csize)] = c.at(i);
        }
        return cc;
    }
    return c;
}

void
EditableDenseThreeDimensionalModel::setColumn(int index,
                                              const Column &values)
{
    QWriteLocker locker(&m_lock);

    while (int(index) >= m_data.size()) {
	m_data.push_back(Column());
        m_trunc.push_back(0);
    }

    bool allChange = false;

//    if (values.size() > m_yBinCount) m_yBinCount = values.size();

    for (int i = 0; i < values.size(); ++i) {
        float value = values[i];
        if (ISNAN(value) || ISINF(value)) {
            continue;
        }
	if (!m_haveExtents || value < m_minimum) {
	    m_minimum = value;
	    allChange = true;
	}
	if (!m_haveExtents || value > m_maximum) {
	    m_maximum = value;
	    allChange = true;
	}
        m_haveExtents = true;
    }

    truncateAndStore(index, values);

//    assert(values == expandAndRetrieve(index));

    long windowStart = index;
    windowStart *= m_resolution;

    if (m_notifyOnAdd) {
	if (allChange) {
	    emit modelChanged();
	} else {
	    emit modelChangedWithin(windowStart, windowStart + m_resolution);
	}
    } else {
	if (allChange) {
	    m_sinceLastNotifyMin = -1;
	    m_sinceLastNotifyMax = -1;
	    emit modelChanged();
	} else {
	    if (m_sinceLastNotifyMin == -1 ||
		windowStart < m_sinceLastNotifyMin) {
		m_sinceLastNotifyMin = windowStart;
	    }
	    if (m_sinceLastNotifyMax == -1 ||
		windowStart > m_sinceLastNotifyMax) {
		m_sinceLastNotifyMax = windowStart;
	    }
	}
    }
}

QString
EditableDenseThreeDimensionalModel::getBinName(int n) const
{
    if (n >= 0 && (int)m_binNames.size() > n) return m_binNames[n];
    else return "";
}

void
EditableDenseThreeDimensionalModel::setBinName(int n, QString name)
{
    while ((int)m_binNames.size() <= n) m_binNames.push_back("");
    m_binNames[n] = name;
    emit modelChanged();
}

void
EditableDenseThreeDimensionalModel::setBinNames(std::vector<QString> names)
{
    m_binNames = names;
    emit modelChanged();
}

bool
EditableDenseThreeDimensionalModel::hasBinValues() const
{
    return !m_binValues.empty();
}

float
EditableDenseThreeDimensionalModel::getBinValue(int n) const
{
    if (n < (int)m_binValues.size()) return m_binValues[n];
    else return 0.f;
}

void
EditableDenseThreeDimensionalModel::setBinValues(std::vector<float> values)
{
    m_binValues = values;
}

QString
EditableDenseThreeDimensionalModel::getBinValueUnit() const
{
    return m_binValueUnit;
}

void
EditableDenseThreeDimensionalModel::setBinValueUnit(QString unit)
{
    m_binValueUnit = unit;
}

bool
EditableDenseThreeDimensionalModel::shouldUseLogValueScale() const
{
    QReadLocker locker(&m_lock);

    vector<double> sample;
    vector<int> n;
    
    for (int i = 0; i < 10; ++i) {
        int index = i * 10;
        if (index < m_data.size()) {
            const Column &c = m_data.at(index);
            while (c.size() > int(sample.size())) {
                sample.push_back(0.0);
                n.push_back(0);
            }
            for (int j = 0; j < c.size(); ++j) {
                sample[j] += c.at(j);
                ++n[j];
            }
        }
    }

    if (sample.empty()) return false;
    for (decltype(sample)::size_type j = 0; j < sample.size(); ++j) {
        if (n[j]) sample[j] /= n[j];
    }
    
    return LogRange::useLogScale(sample);
}

void
EditableDenseThreeDimensionalModel::setCompletion(int completion, bool update)
{
    if (m_completion != completion) {
	m_completion = completion;

	if (completion == 100) {

	    m_notifyOnAdd = true; // henceforth
	    emit modelChanged();

	} else if (!m_notifyOnAdd) {

	    if (update &&
                m_sinceLastNotifyMin >= 0 &&
		m_sinceLastNotifyMax >= 0) {
		emit modelChangedWithin(m_sinceLastNotifyMin,
                                        m_sinceLastNotifyMax + m_resolution);
		m_sinceLastNotifyMin = m_sinceLastNotifyMax = -1;
	    } else {
		emit completionChanged();
	    }
	} else {
	    emit completionChanged();
	}	    
    }
}

QString
EditableDenseThreeDimensionalModel::toDelimitedDataString(QString delimiter) const
{
    QReadLocker locker(&m_lock);
    QString s;
    for (int i = 0; i < m_data.size(); ++i) {
        QStringList list;
	for (int j = 0; j < m_data.at(i).size(); ++j) {
            list << QString("%1").arg(m_data.at(i).at(j));
        }
        s += list.join(delimiter) + "\n";
    }
    return s;
}

QString
EditableDenseThreeDimensionalModel::toDelimitedDataStringSubset(QString delimiter, sv_frame_t f0, sv_frame_t f1) const
{
    QReadLocker locker(&m_lock);
    QString s;
    for (int i = 0; i < m_data.size(); ++i) {
        sv_frame_t fr = m_startFrame + i * m_resolution;
        if (fr >= f0 && fr < f1) {
            QStringList list;
            for (int j = 0; j < m_data.at(i).size(); ++j) {
                list << QString("%1").arg(m_data.at(i).at(j));
            }
            s += list.join(delimiter) + "\n";
        }
    }
    return s;
}

void
EditableDenseThreeDimensionalModel::toXml(QTextStream &out,
                                          QString indent,
                                          QString extraAttributes) const
{
    QReadLocker locker(&m_lock);

    // For historical reasons we read and write "resolution" as "windowSize"

    cerr << "EditableDenseThreeDimensionalModel::toXml" << endl;

    Model::toXml
	(out, indent,
         QString("type=\"dense\" dimensions=\"3\" windowSize=\"%1\" yBinCount=\"%2\" minimum=\"%3\" maximum=\"%4\" dataset=\"%5\" startFrame=\"%6\" %7")
	 .arg(m_resolution)
	 .arg(m_yBinCount)
	 .arg(m_minimum)
	 .arg(m_maximum)
	 .arg(getObjectExportId(&m_data))
         .arg(m_startFrame)
	 .arg(extraAttributes));

    out << indent;
    out << QString("<dataset id=\"%1\" dimensions=\"3\" separator=\" \">\n")
	.arg(getObjectExportId(&m_data));

    for (int i = 0; i < (int)m_binNames.size(); ++i) {
	if (m_binNames[i] != "") {
	    out << indent + "  ";
	    out << QString("<bin number=\"%1\" name=\"%2\"/>\n")
		.arg(i).arg(m_binNames[i]);
	}
    }

    for (int i = 0; i < (int)m_data.size(); ++i) {
	out << indent + "  ";
	out << QString("<row n=\"%1\">").arg(i);
	for (int j = 0; j < (int)m_data.at(i).size(); ++j) {
	    if (j > 0) out << " ";
	    out << m_data.at(i).at(j);
	}
	out << QString("</row>\n");
        out.flush();
    }

    out << indent + "</dataset>\n";
}


