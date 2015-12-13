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

#ifndef _WAV_FILE_WRITER_H_
#define _WAV_FILE_WRITER_H_

#include <QString>

#include <sndfile.h>

#include "base/BaseTypes.h"

class DenseTimeValueModel;
class MultiSelection;
class TempWriteFile;

class WavFileWriter
{
public:
    /**
     * Specify the method used to open the destination file.
     * 
     * If WriteToTemporary, the destination will be opened as a
     * temporary file which is moved to the target location when the
     * WavFileWriter is closed or deleted (to avoid clobbering an
     * existing file with a partially written replacement).
     * 
     * If WriteToTarget, the target file will be opened directly
     * (necessary when e.g. doing a series of incremental writes to a
     * file while keeping it open for reading).
     */
    enum FileWriteMode {
        WriteToTemporary,
        WriteToTarget
    };

    WavFileWriter(QString path, sv_samplerate_t sampleRate, int channels,
                  FileWriteMode mode);
    virtual ~WavFileWriter();

    bool isOK() const;

    virtual QString getError() const;

    QString getPath() const { return m_path; }

    bool writeModel(DenseTimeValueModel *source,
                    MultiSelection *selection = 0);

    bool writeSamples(float **samples, sv_frame_t count); // count per channel

    bool close();

protected:
    QString m_path;
    sv_samplerate_t m_sampleRate;
    int m_channels;
    TempWriteFile *m_temp;
    SNDFILE *m_file;
    QString m_error;

    QString getWriteFilename() const;
};


#endif
