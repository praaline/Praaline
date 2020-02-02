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

#ifndef _CSV_FORMAT_H_
#define _CSV_FORMAT_H_

#include <QString>
#include <QStringList>

#include "PraalineCore/Base/BaseTypes.h"

class CSVFormat
{
public:
    enum ModelType {
	OneDimensionalModel,
	TwoDimensionalModel,
        TwoDimensionalModelWithDuration,
        TwoDimensionalModelWithDurationAndPitch,
	ThreeDimensionalModel
    };
    
    enum TimingType {
	ExplicitTiming,
	ImplicitTiming
    };

    enum TimeUnits {
	TimeSeconds,
        TimeMilliseconds,
	TimeAudioFrames,
	TimeWindows,
    };

    enum ColumnPurpose {
        ColumnUnknown,
        ColumnStartTime,
        ColumnEndTime,
        ColumnDuration,
        ColumnValue,
        ColumnPitch,
        ColumnLabel
    };

    enum ColumnQuality {
        ColumnNumeric    = 1,
        ColumnIntegral   = 2,
        ColumnIncreasing = 4,
        ColumnLarge      = 8,
        ColumnNearEmpty  = 16,
    };
    typedef unsigned int ColumnQualities;

    CSVFormat() : // arbitrary defaults
        m_modelType(TwoDimensionalModel),
        m_timingType(ExplicitTiming),
        m_timeUnits(TimeSeconds),
        m_separator(","),
        m_sampleRate(44100),
        m_windowSize(1024),
        m_columnCount(0),
        m_variableColumnCount(false),
        m_allowQuoting(true),
        m_maxExampleCols(0)
    { }

    CSVFormat(QString path); // guess format

    /**
     * Guess the format of the given CSV file, setting the fields in
     * this object accordingly.  If the current separator is the empty
     * string, the separator character will also be guessed; otherwise
     * the current separator will be used.  The other properties of
     * this object will be set according to guesses from the file.
     */
    void guessFormatFor(QString path);
 
    ModelType    getModelType()     const { return m_modelType;     }
    TimingType   getTimingType()    const { return m_timingType;    }
    TimeUnits    getTimeUnits()     const { return m_timeUnits;     }
    sv_samplerate_t getSampleRate() const { return m_sampleRate;    }
    int          getWindowSize()    const { return m_windowSize;    }
    int          getColumnCount()   const { return m_columnCount;   }
    bool         getAllowQuoting()  const { return m_allowQuoting;  }
    QChar        getSeparator()     const { 
        if (m_separator == "") return ' ';
        else return m_separator[0];
    }

    void setModelType(ModelType t)        { m_modelType    = t; }
    void setTimingType(TimingType t)      { m_timingType   = t; }
    void setTimeUnits(TimeUnits t)        { m_timeUnits    = t; }
    void setSeparator(QChar s)            { m_separator    = s; }
    void setSampleRate(sv_samplerate_t r) { m_sampleRate   = r; }
    void setWindowSize(int s)             { m_windowSize   = s; }
    void setColumnCount(int c)            { m_columnCount  = c; }
    void setAllowQuoting(bool q)          { m_allowQuoting = q; }

    QList<ColumnPurpose> getColumnPurposes() const { return m_columnPurposes; }
    void setColumnPurposes(QList<ColumnPurpose> cl) { m_columnPurposes = cl; }

    ColumnPurpose getColumnPurpose(int i);
    ColumnPurpose getColumnPurpose(int i) const;
    void setColumnPurpose(int i, ColumnPurpose p);
    
    // read-only; only valid if format has been guessed:
    QList<ColumnQualities> getColumnQualities() const { return m_columnQualities; }

    // read-only; only valid if format has been guessed:
    QList<QStringList> getExample() const { return m_example; }
    int getMaxExampleCols() const { return m_maxExampleCols; }
	
protected:
    ModelType    m_modelType;
    TimingType   m_timingType;
    TimeUnits    m_timeUnits;
    QString      m_separator;
    sv_samplerate_t m_sampleRate;
    int          m_windowSize;

    int          m_columnCount;
    bool         m_variableColumnCount;

    QList<ColumnQualities> m_columnQualities;
    QList<ColumnPurpose> m_columnPurposes;

    QList<float> m_prevValues;

    bool m_allowQuoting;

    QList<QStringList> m_example;
    int m_maxExampleCols;

    void guessSeparator(QString line);
    void guessQualities(QString line, int lineno);
    void guessPurposes();

    void guessFormatFor_Old(QString path);
 
};

#endif
