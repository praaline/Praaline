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

#include "CSVFileReader.h"

#include "model/Model.h"
#include "base/RealTime.h"
#include "base/StringBits.h"
#include "model/SparseOneDimensionalModel.h"
#include "model/SparseTimeValueModel.h"
#include "model/EditableDenseThreeDimensionalModel.h"
#include "model/RegionModel.h"
#include "model/NoteModel.h"
#include "DataFileReaderFactory.h"

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>

#include <iostream>
#include <map>

CSVFileReader::CSVFileReader(QString path, CSVFormat format,
                             sv_samplerate_t mainModelSampleRate) :
    m_format(format),
    m_device(0),
    m_ownDevice(true),
    m_warnings(0),
    m_mainModelSampleRate(mainModelSampleRate)
{
    QFile *file = new QFile(path);
    bool good = false;
    
    if (!file->exists()) {
	m_error = QFile::tr("File \"%1\" does not exist").arg(path);
    } else if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
	m_error = QFile::tr("Failed to open file \"%1\"").arg(path);
    } else {
	good = true;
    }

    if (good) {
        m_device = file;
        m_filename = QFileInfo(path).fileName();
    } else {
	delete file;
    }
}

CSVFileReader::CSVFileReader(QIODevice *device, CSVFormat format,
                             sv_samplerate_t mainModelSampleRate) :
    m_format(format),
    m_device(device),
    m_ownDevice(false),
    m_warnings(0),
    m_mainModelSampleRate(mainModelSampleRate)
{
}

CSVFileReader::~CSVFileReader()
{
    cerr << "CSVFileReader::~CSVFileReader: device is " << m_device << endl;

    if (m_device && m_ownDevice) {
        cerr << "CSVFileReader::CSVFileReader: Closing device" << endl;
        m_device->close();
        delete m_device;
    }
}

bool
CSVFileReader::isOK() const
{
    return (m_device != 0);
}

QString
CSVFileReader::getError() const
{
    return m_error;
}

sv_frame_t
CSVFileReader::convertTimeValue(QString s, int lineno,
                                sv_samplerate_t sampleRate,
                                int windowSize) const
{
    QRegExp nonNumericRx("[^0-9eE.,+-]");
    int warnLimit = 10;

    CSVFormat::TimeUnits timeUnits = m_format.getTimeUnits();

    sv_frame_t calculatedFrame = 0;

    bool ok = false;
    QString numeric = s;
    numeric.remove(nonNumericRx);
    
    if (timeUnits == CSVFormat::TimeSeconds) {

        double time = numeric.toDouble(&ok);
        if (!ok) time = StringBits::stringToDoubleLocaleFree(numeric, &ok);
        calculatedFrame = sv_frame_t(time * sampleRate + 0.5);
    
    } else if (timeUnits == CSVFormat::TimeMilliseconds) {

        double time = numeric.toDouble(&ok);
        if (!ok) time = StringBits::stringToDoubleLocaleFree(numeric, &ok);
        calculatedFrame = sv_frame_t((time / 1000.0) * sampleRate + 0.5);
        
    } else {
        
        long n = numeric.toLong(&ok);
        if (n >= 0) calculatedFrame = n;
        
        if (timeUnits == CSVFormat::TimeWindows) {
            calculatedFrame *= windowSize;
        }
    }
    
    if (!ok) {
        if (m_warnings < warnLimit) {
            cerr << "WARNING: CSVFileReader::load: "
                      << "Bad time format (\"" << s
                      << "\") in data line "
                      << lineno+1 << endl;
        } else if (m_warnings == warnLimit) {
            cerr << "WARNING: Too many warnings" << endl;
        }
        ++m_warnings;
    }

    return calculatedFrame;
}

Model *
CSVFileReader::load() const
{
    if (!m_device) return 0;

    CSVFormat::ModelType modelType = m_format.getModelType();
    CSVFormat::TimingType timingType = m_format.getTimingType();
    CSVFormat::TimeUnits timeUnits = m_format.getTimeUnits();
    sv_samplerate_t sampleRate = m_format.getSampleRate();
    int windowSize = m_format.getWindowSize();
    QChar separator = m_format.getSeparator();
    bool allowQuoting = m_format.getAllowQuoting();

    if (timingType == CSVFormat::ExplicitTiming) {
        if (modelType == CSVFormat::ThreeDimensionalModel) {
            // This will be overridden later if more than one line
            // appears in our file, but we want to choose a default
            // that's likely to be visible
            windowSize = 1024;
        } else {
            windowSize = 1;
        }
	if (timeUnits == CSVFormat::TimeSeconds ||
            timeUnits == CSVFormat::TimeMilliseconds) {
	    sampleRate = m_mainModelSampleRate;
	}
    }

    SparseOneDimensionalModel *model1 = 0;
    SparseTimeValueModel *model2 = 0;
    RegionModel *model2a = 0;
    NoteModel *model2b = 0;
    EditableDenseThreeDimensionalModel *model3 = 0;
    Model *model = 0;

    QTextStream in(m_device);

    unsigned int warnings = 0, warnLimit = 10;
    unsigned int lineno = 0;

    float min = 0.0, max = 0.0;

    sv_frame_t frameNo = 0;
    sv_frame_t duration = 0;
    sv_frame_t endFrame = 0;

    bool haveAnyValue = false;
    bool haveEndTime = false;
    bool pitchLooksLikeMIDI = true;

    sv_frame_t startFrame = 0; // for calculation of dense model resolution
    bool firstEverValue = true;

    std::map<QString, int> labelCountMap;
    
    int valueColumns = 0;
    for (int i = 0; i < m_format.getColumnCount(); ++i) {
        if (m_format.getColumnPurpose(i) == CSVFormat::ColumnValue) {
            ++valueColumns;
        }
    }

    while (!in.atEnd()) {

        // QTextStream's readLine doesn't cope with old-style Mac
        // CR-only line endings.  Why did they bother making the class
        // cope with more than one sort of line ending, if it still
        // can't be configured to cope with all the common sorts?

        // For the time being we'll deal with this case (which is
        // relatively uncommon for us, but still necessary to handle)
        // by reading the entire file using a single readLine, and
        // splitting it.  For CR and CR/LF line endings this will just
        // read a line at a time, and that's obviously OK.

        QString chunk = in.readLine();
        QStringList lines = chunk.split('\r', QString::SkipEmptyParts);
        
        for (int li = 0; li < lines.size(); ++li) {

            QString line = lines[li];
            
            if (line.startsWith("#")) continue;

            QStringList list = StringBits::split(line, separator, allowQuoting);
            if (!model) {

                switch (modelType) {

                case CSVFormat::OneDimensionalModel:
                    model1 = new SparseOneDimensionalModel(sampleRate, windowSize);
                    model = model1;
                    break;
		
                case CSVFormat::TwoDimensionalModel:
                    model2 = new SparseTimeValueModel(sampleRate, windowSize, false);
                    model = model2;
                    break;
		
                case CSVFormat::TwoDimensionalModelWithDuration:
                    model2a = new RegionModel(sampleRate, windowSize, false);
                    model = model2a;
                    break;
		
                case CSVFormat::TwoDimensionalModelWithDurationAndPitch:
                    model2b = new NoteModel(sampleRate, windowSize, false);
                    model = model2b;
                    break;
		
                case CSVFormat::ThreeDimensionalModel:
                    model3 = new EditableDenseThreeDimensionalModel
                        (sampleRate,
                         windowSize,
                         valueColumns,
                         EditableDenseThreeDimensionalModel::NoCompression);
                    model = model3;
                    break;
                }

                if (model) {
                    if (m_filename != "") {
                        model->setObjectName(m_filename);
                    }
                }
            }

            float value = 0.f;
            float pitch = 0.f;
            QString label = "";

            duration = 0.f;
            haveEndTime = false;

            for (int i = 0; i < list.size(); ++i) {

                QString s = list[i];

                CSVFormat::ColumnPurpose purpose = m_format.getColumnPurpose(i);

                switch (purpose) {

                case CSVFormat::ColumnUnknown:
                    break;

                case CSVFormat::ColumnStartTime:
                    frameNo = convertTimeValue(s, lineno, sampleRate, windowSize);
                    break;
                
                case CSVFormat::ColumnEndTime:
                    endFrame = convertTimeValue(s, lineno, sampleRate, windowSize);
                    haveEndTime = true;
                    break;

                case CSVFormat::ColumnDuration:
                    duration = convertTimeValue(s, lineno, sampleRate, windowSize);
                    break;

                case CSVFormat::ColumnValue:
                    value = s.toFloat();
                    haveAnyValue = true;
                    break;

                case CSVFormat::ColumnPitch:
                    pitch = s.toFloat();
                    if (pitch < 0.f || pitch > 127.f) {
                        pitchLooksLikeMIDI = false;
                    }
                    break;

                case CSVFormat::ColumnLabel:
                    label = s;
                    ++labelCountMap[label];
                    break;
                }
            }

            if (haveEndTime) { // ... calculate duration now all cols read
                if (endFrame > frameNo) {
                    duration = endFrame - frameNo;
                }
            }

            if (modelType == CSVFormat::OneDimensionalModel) {
	    
                SparseOneDimensionalModel::Point point(frameNo, label);
                model1->addPoint(point);

            } else if (modelType == CSVFormat::TwoDimensionalModel) {

                SparseTimeValueModel::Point point(frameNo, value, label);
                model2->addPoint(point);

            } else if (modelType == CSVFormat::TwoDimensionalModelWithDuration) {

                RegionModel::Point point(frameNo, value, duration, label);
                model2a->addPoint(point);

            } else if (modelType == CSVFormat::TwoDimensionalModelWithDurationAndPitch) {

                float level = ((value >= 0.f && value <= 1.f) ? value : 1.f);
                NoteModel::Point point(frameNo, pitch, duration, level, label);
                model2b->addPoint(point);

            } else if (modelType == CSVFormat::ThreeDimensionalModel) {

                DenseThreeDimensionalModel::Column values;

                for (int i = 0; i < list.size(); ++i) {

                    if (m_format.getColumnPurpose(i) != CSVFormat::ColumnValue) {
                        continue;
                    }

                    bool ok = false;
                    float value = list[i].toFloat(&ok);

                    values.push_back(value);
	    
                    if (firstEverValue || value < min) min = value;
                    if (firstEverValue || value > max) max = value;
                    
                    if (firstEverValue) {
                        startFrame = frameNo;
                        model3->setStartFrame(startFrame);
                    } else if (lineno == 1 &&
                               timingType == CSVFormat::ExplicitTiming) {
                        model3->setResolution(int(frameNo - startFrame));
                    }
                    
                    firstEverValue = false;

                    if (!ok) {
                        if (warnings < warnLimit) {
                            cerr << "WARNING: CSVFileReader::load: "
                                      << "Non-numeric value \""
                                      << list[i]
                                      << "\" in data line " << lineno+1
                                      << ":" << endl;
                            cerr << line << endl;
                            ++warnings;
                        } else if (warnings == warnLimit) {
//                            cerr << "WARNING: Too many warnings" << endl;
                        }
                    }
                }
	
//                cerr << "Setting bin values for count " << lineno << ", frame "
//                          << frameNo << ", time " << RealTime::frame2RealTime(frameNo, sampleRate) << endl;

                model3->setColumn(lineno, values);
            }

            ++lineno;
            if (timingType == CSVFormat::ImplicitTiming ||
                list.size() == 0) {
                frameNo += windowSize;
            }
        }
    }

    if (!haveAnyValue) {
        if (model2a) {
            // assign values for regions based on label frequency; we
            // have this in our labelCountMap, sort of

            std::map<int, std::map<QString, float> > countLabelValueMap;
            for (std::map<QString, int>::iterator i = labelCountMap.begin();
                 i != labelCountMap.end(); ++i) {
                countLabelValueMap[i->second][i->first] = 0.f;
            }

            float v = 0.f;
            for (std::map<int, std::map<QString, float> >::iterator i =
                     countLabelValueMap.end(); i != countLabelValueMap.begin(); ) {
                --i;
                for (std::map<QString, float>::iterator j = i->second.begin();
                     j != i->second.end(); ++j) {
                    j->second = v;
                    v = v + 1.f;
                }
            }

            std::map<RegionModel::Point, RegionModel::Point,
                RegionModel::Point::Comparator> pointMap;
            for (RegionModel::PointList::const_iterator i =
                     model2a->getPoints().begin();
                 i != model2a->getPoints().end(); ++i) {
                RegionModel::Point p(*i);
                v = countLabelValueMap[labelCountMap[p.label]][p.label];
                RegionModel::Point pp(p.frame, v, p.duration, p.label);
                pointMap[p] = pp;
            }

            for (std::map<RegionModel::Point, RegionModel::Point>::iterator i = 
                     pointMap.begin(); i != pointMap.end(); ++i) {
                model2a->deletePoint(i->first);
                model2a->addPoint(i->second);
            }
        }
    }
                
    if (model2b) {
        if (pitchLooksLikeMIDI) {
            model2b->setScaleUnits("MIDI Pitch");
        } else {
            model2b->setScaleUnits("Hz");
        }
    }

    if (model3) {
	model3->setMinimumLevel(min);
	model3->setMaximumLevel(max);
    }

    return model;
}

