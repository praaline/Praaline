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

#ifndef _AUDIO_FILE_READER_FACTORY_H_
#define _AUDIO_FILE_READER_FACTORY_H_

#include <QString>

#include "FileSource.h"
#include "PraalineCore/Base/BaseTypes.h"

class AudioFileReader;
class ProgressReporter;

class AudioFileReaderFactory
{
public:
    /**
     * Return the file extensions that we have audio file readers for,
     * in a format suitable for use with QFileDialog.  For example,
     * "*.wav *.aiff *.ogg".
     */
    static QString getKnownExtensions();

    /**
     * Return an audio file reader initialised to the file at the
     * given path, or NULL if no suitable reader for this path is
     * available or the file cannot be opened.
     *
     * If targetRate is non-zero, the file will be resampled to that
     * rate (transparently).  You can query reader->getNativeRate()
     * if you want to find out whether the file is being resampled
     * or not.
     *
     * If normalised is true, the file data will be normalised to
     * abs(max) == 1.0. Otherwise the file will not be normalised.
     *
     * If a ProgressReporter is provided, it will be updated with
     * progress status.  Caller retains ownership of the reporter
     * object.
     *
     * Caller owns the returned object and must delete it after use.
     */
    static AudioFileReader *createReader(FileSource source,
                                         sv_samplerate_t targetRate = 0,
                                         bool normalised = false,
                                         ProgressReporter *reporter = 0);

    /**
     * Return an audio file reader initialised to the file at the
     * given path, or NULL if no suitable reader for this path is
     * available or the file cannot be opened.  If the reader supports
     * threaded decoding, it will be used and the file decoded in a
     * background thread.
     *
     * If targetRate is non-zero, the file will be resampled to that
     * rate (transparently).  You can query reader->getNativeRate()
     * if you want to find out whether the file is being resampled
     * or not.
     *
     * If normalised is true, the file data will be normalised to
     * abs(max) == 1.0. Otherwise the file will not be normalised.
     *
     * If a ProgressReporter is provided, it will be updated with
     * progress status.  This will only be meaningful if threading
     * mode is not used because the file reader in use does not
     * support it; otherwise progress as reported will jump straight
     * to 100% before threading mode takes over.  Caller retains
     * ownership of the reporter object.
     *
     * Caller owns the returned object and must delete it after use.
     */
    static AudioFileReader *createThreadingReader(FileSource source,
                                                  sv_samplerate_t targetRate = 0,
                                                  bool normalised = false,
                                                  ProgressReporter *reporter = 0);

protected:
    static AudioFileReader *create(FileSource source,
                                   sv_samplerate_t targetRate,
                                   bool normalised,
                                   bool threading,
                                   ProgressReporter *reporter);
};

#endif

