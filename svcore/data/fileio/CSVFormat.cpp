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

#include "CSVFormat.h"

#include "base/StringBits.h"

#include <QFile>
#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>

#include <iostream>

CSVFormat::CSVFormat(QString path) :
    m_separator(""),
    m_sampleRate(44100),
    m_windowSize(1024),
    m_allowQuoting(true)
{
    guessFormatFor(path);
}

void
CSVFormat::guessFormatFor(QString path)
{
    m_modelType = TwoDimensionalModel;
    m_timingType = ExplicitTiming;
    m_timeUnits = TimeSeconds;

    m_maxExampleCols = 0;
    m_columnCount = 0;
    m_variableColumnCount = false;

    m_example.clear();
    m_columnQualities.clear();
    m_columnPurposes.clear();
    m_prevValues.clear();

    QFile file(path);
    if (!file.exists()) return;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    in.seek(0);

    int lineno = 0;

    while (!in.atEnd()) {

        // See comment about line endings in CSVFileReader::load() 

        QString chunk = in.readLine();
        QStringList lines = chunk.split('\r', QString::SkipEmptyParts);

        for (int li = 0; li < lines.size(); ++li) {

            QString line = lines[li];
            if (line.startsWith("#") || line == "") continue;

            guessQualities(line, lineno);

            ++lineno;
        }

        if (lineno >= 50) break;
    }

    guessPurposes();
}

void
CSVFormat::guessSeparator(QString line)
{
    char candidates[] = { ',', '\t', ' ', '|', '/', ':' };
    for (int i = 0; i < int(sizeof(candidates)/sizeof(candidates[0])); ++i) {
        if (StringBits::split(line, candidates[i], m_allowQuoting).size() >= 2) {
            m_separator = candidates[i];
            return;
        }
    }
    m_separator = " ";
}

void
CSVFormat::guessQualities(QString line, int lineno)
{
    if (m_separator == "") guessSeparator(line);

    QStringList list = StringBits::split(line, m_separator[0], m_allowQuoting);

    int cols = list.size();
    if (lineno == 0 || (cols > m_columnCount)) m_columnCount = cols;
    if (cols != m_columnCount) m_variableColumnCount = true;

    // All columns are regarded as having these qualities until we see
    // something that indicates otherwise:

    ColumnQualities defaultQualities =
        ColumnNumeric | ColumnIntegral | ColumnIncreasing | ColumnNearEmpty;
    
    for (int i = 0; i < cols; ++i) {
	    
        while (m_columnQualities.size() <= i) {
            m_columnQualities.push_back(defaultQualities);
            m_prevValues.push_back(0.f);
        }

        QString s(list[i]);
        bool ok = false;

        ColumnQualities qualities = m_columnQualities[i];

        bool numeric    = (qualities & ColumnNumeric);
        bool integral   = (qualities & ColumnIntegral);
        bool increasing = (qualities & ColumnIncreasing);
        bool large      = (qualities & ColumnLarge); // this one defaults to off
        bool emptyish   = (qualities & ColumnNearEmpty);

        if (lineno > 1 && s.trimmed() != "") {
            emptyish = false;
        }
        
        float value = 0.f;

        //!!! how to take into account headers?

        if (numeric) {
            value = s.toFloat(&ok);
            if (!ok) {
                value = (float)StringBits::stringToDoubleLocaleFree(s, &ok);
            }
            if (ok) {
                if (lineno < 2 && value > 1000.f) large = true;
            } else {
                numeric = false;
            }
        }

        if (numeric) {

            if (integral) {
                if (s.contains('.') || s.contains(',')) {
                    integral = false;
                }
            }

            if (increasing) {
                if (lineno > 0 && value <= m_prevValues[i]) {
                    increasing = false;
                }
            }

            m_prevValues[i] = value;
        }

        m_columnQualities[i] =
            (numeric    ? ColumnNumeric : 0) |
            (integral   ? ColumnIntegral : 0) |
            (increasing ? ColumnIncreasing : 0) |
            (large      ? ColumnLarge : 0) |
            (emptyish   ? ColumnNearEmpty : 0);
    }

    if (lineno < 10) {
        m_example.push_back(list);
        if (lineno == 0 || cols > m_maxExampleCols) {
            m_maxExampleCols = cols;
        }
    }

//    cerr << "Estimated column qualities: ";
//    for (int i = 0; i < m_columnCount; ++i) {
//        cerr << int(m_columnQualities[i]) << " ";
//    }
//    cerr << endl;
}

void
CSVFormat::guessPurposes()
{
    m_timingType = CSVFormat::ImplicitTiming;
    m_timeUnits = CSVFormat::TimeWindows;
	
    int timingColumnCount = 0;

    // if our first column has zero or one entries in it and the rest
    // have more, then we'll default to ignoring the first column and
    // counting the next one as primary. (e.g. Sonic Annotator output
    // with filename at start of first column.)

    int primaryColumnNo = 0;

    if (m_columnCount >= 2) {
        if ( (m_columnQualities[0] & ColumnNearEmpty) &&
            !(m_columnQualities[1] & ColumnNearEmpty)) {
            primaryColumnNo = 1;
        }
    }
    
    for (int i = 0; i < m_columnCount; ++i) {
        
        ColumnPurpose purpose = ColumnUnknown;

        if (i < primaryColumnNo) {
            setColumnPurpose(i, purpose);
            continue;
        }
        
        bool primary = (i == primaryColumnNo);

        ColumnQualities qualities = m_columnQualities[i];

        bool numeric    = (qualities & ColumnNumeric);
        bool integral   = (qualities & ColumnIntegral);
        bool increasing = (qualities & ColumnIncreasing);
        bool large      = (qualities & ColumnLarge);

        bool timingColumn = (numeric && increasing);

        if (timingColumn) {

            ++timingColumnCount;
                              
            if (primary) {

                purpose = ColumnStartTime;

                m_timingType = ExplicitTiming;

                if (integral && large) {
                    m_timeUnits = TimeAudioFrames;
                } else {
                    m_timeUnits = TimeSeconds;
                }

            } else {

                if (timingColumnCount == 2 && m_timingType == ExplicitTiming) {
                    purpose = ColumnEndTime;
                }
            }
        }

        if (purpose == ColumnUnknown) {
            if (numeric) {
                purpose = ColumnValue;
            } else {
                purpose = ColumnLabel;
            }
        }

        setColumnPurpose(i, purpose);
    }            

    int valueCount = 0;
    for (int i = 0; i < m_columnCount; ++i) {
        if (m_columnPurposes[i] == ColumnValue) ++valueCount;
    }

    if (valueCount == 2 && timingColumnCount == 1) {
        // If we have exactly two apparent value columns and only one
        // timing column, but one value column is integral and the
        // other is not, guess that whichever one matches the integral
        // status of the time column is either duration or end time
        if (m_timingType == ExplicitTiming) {
            int a = -1, b = -1;
            for (int i = 0; i < m_columnCount; ++i) {
                if (m_columnPurposes[i] == ColumnValue) {
                    if (a == -1) a = i;
                    else b = i;
                }
            }
            if ((m_columnQualities[a] & ColumnIntegral) !=
                (m_columnQualities[b] & ColumnIntegral)) {
                int timecol = a;
                if ((m_columnQualities[a] & ColumnIntegral) !=
                    (m_columnQualities[0] & ColumnIntegral)) {
                    timecol = b;
                }
                if (m_columnQualities[timecol] & ColumnIncreasing) {
                    // This shouldn't happen; should have been settled above
                    m_columnPurposes[timecol] = ColumnEndTime;
                } else {
                    m_columnPurposes[timecol] = ColumnDuration;
                }
                --valueCount;
            }
        }
    }

    if (timingColumnCount > 1) {
        m_modelType = TwoDimensionalModelWithDuration;
    } else {
        if (valueCount == 0) {
            m_modelType = OneDimensionalModel;
        } else if (valueCount == 1) {
            m_modelType = TwoDimensionalModel;
        } else {
            m_modelType = ThreeDimensionalModel;
        }
    }

//    cerr << "Estimated column purposes: ";
//    for (int i = 0; i < m_columnCount; ++i) {
//        cerr << int(m_columnPurposes[i]) << " ";
//    }
//    cerr << endl;

//    cerr << "Estimated model type: " << m_modelType << endl;
//    cerr << "Estimated timing type: " << m_timingType << endl;
//    cerr << "Estimated units: " << m_timeUnits << endl;
}

CSVFormat::ColumnPurpose
CSVFormat::getColumnPurpose(int i)
{
    while (m_columnPurposes.size() <= i) {
        m_columnPurposes.push_back(ColumnUnknown);
    }
    return m_columnPurposes[i];
}

CSVFormat::ColumnPurpose
CSVFormat::getColumnPurpose(int i) const
{
    if (m_columnPurposes.size() <= i) {
        return ColumnUnknown;
    }
    return m_columnPurposes[i];
}

void
CSVFormat::setColumnPurpose(int i, ColumnPurpose p)
{
    while (m_columnPurposes.size() <= i) {
        m_columnPurposes.push_back(ColumnUnknown);
    }
    m_columnPurposes[i] = p;
}




