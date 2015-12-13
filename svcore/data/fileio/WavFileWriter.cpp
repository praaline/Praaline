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

#include "WavFileWriter.h"

#include "model/DenseTimeValueModel.h"
#include "base/Selection.h"
#include "base/TempWriteFile.h"
#include "base/Exceptions.h"

#include <QFileInfo>

#include <iostream>
#include <cmath>

WavFileWriter::WavFileWriter(QString path,
			     sv_samplerate_t sampleRate,
                             int channels,
                             FileWriteMode mode) :
    m_path(path),
    m_sampleRate(sampleRate),
    m_channels(channels),
    m_temp(0),
    m_file(0)
{
    SF_INFO fileInfo;

    int fileRate = int(round(m_sampleRate));
    if (m_sampleRate != sv_samplerate_t(fileRate)) {
        cerr << "WavFileWriter: WARNING: Non-integer sample rate "
             << m_sampleRate << " presented, rounding to " << fileRate
             << endl;
    }
    fileInfo.samplerate = fileRate;
    fileInfo.channels = m_channels;
    fileInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    try {
        if (mode == WriteToTemporary) {
            m_temp = new TempWriteFile(m_path);
            m_file = sf_open(m_temp->getTemporaryFilename().toLocal8Bit(),
                             SFM_WRITE, &fileInfo);
            if (!m_file) {
                cerr << "WavFileWriter: Failed to open file ("
                          << sf_strerror(m_file) << ")" << endl;
                m_error = QString("Failed to open audio file '%1' for writing")
                    .arg(m_temp->getTemporaryFilename());
            }
        } else {
            m_file = sf_open(m_path.toLocal8Bit(), SFM_WRITE, &fileInfo);
            if (!m_file) {
                cerr << "WavFileWriter: Failed to open file ("
                          << sf_strerror(m_file) << ")" << endl;
                m_error = QString("Failed to open audio file '%1' for writing")
                    .arg(m_path);
            }
        }            
    } catch (FileOperationFailed &f) {
        m_error = f.what();
        m_temp = 0;
        m_file = 0;
    }
}

WavFileWriter::~WavFileWriter()
{
    if (m_file) close();
}

bool
WavFileWriter::isOK() const
{
    return (m_error.isEmpty());
}

QString
WavFileWriter::getError() const
{
    return m_error;
}

QString
WavFileWriter::getWriteFilename() const
{
    if (m_temp) {
        return m_temp->getTemporaryFilename();
    } else {
        return m_path;
    }
}

bool
WavFileWriter::writeModel(DenseTimeValueModel *source,
                          MultiSelection *selection)
{
    if (source->getChannelCount() != m_channels) {
        cerr << "WavFileWriter::writeModel: Wrong number of channels ("
                  << source->getChannelCount()  << " != " << m_channels << ")"
                  << endl;
        m_error = QString("Failed to write model to audio file '%1'")
            .arg(getWriteFilename());
        return false;
    }

    if (!m_file) {
        m_error = QString("Failed to write model to audio file '%1': File not open")
            .arg(getWriteFilename());
	return false;
    }

    bool ownSelection = false;
    if (!selection) {
	selection = new MultiSelection;
	selection->setSelection(Selection(source->getStartFrame(),
					  source->getEndFrame()));
        ownSelection = true;
    }

    sv_frame_t bs = 2048;
    float *ub = new float[bs]; // uninterleaved buffer (one channel)
    float *ib = new float[bs * m_channels]; // interleaved buffer

    for (MultiSelection::SelectionList::iterator i =
	     selection->getSelections().begin();
	 i != selection->getSelections().end(); ++i) {
	
	sv_frame_t f0(i->getStartFrame()), f1(i->getEndFrame());

	for (sv_frame_t f = f0; f < f1; f += bs) {
	    
	    sv_frame_t n = std::min(bs, f1 - f);

	    for (int c = 0; c < int(m_channels); ++c) {
		source->getData(c, f, n, ub);
		for (int i = 0; i < n; ++i) {
		    ib[i * m_channels + c] = ub[i];
		}
	    }	    

	    sf_count_t written = sf_writef_float(m_file, ib, n);

	    if (written < n) {
		m_error = QString("Only wrote %1 of %2 frames at file frame %3")
		    .arg(written).arg(n).arg(f);
		break;
	    }
	}
    }

    delete[] ub;
    delete[] ib;
    if (ownSelection) delete selection;

    return isOK();
}
	
bool
WavFileWriter::writeSamples(float **samples, sv_frame_t count)
{
    if (!m_file) {
        m_error = QString("Failed to write model to audio file '%1': File not open")
            .arg(getWriteFilename());
	return false;
    }

    float *b = new float[count * m_channels];
    for (sv_frame_t i = 0; i < count; ++i) {
        for (int c = 0; c < int(m_channels); ++c) {
            b[i * m_channels + c] = samples[c][i];
        }
    }

    sv_frame_t written = sf_writef_float(m_file, b, count);

    delete[] b;

    if (written < count) {
        m_error = QString("Only wrote %1 of %2 frames")
            .arg(written).arg(count);
    }

    return isOK();
}
    
bool
WavFileWriter::close()
{
    if (m_file) {
        sf_close(m_file);
        m_file = 0;
    }
    if (m_temp) {
        m_temp->moveToTarget();
        delete m_temp;
        m_temp = 0;
    }
    return true;
}

