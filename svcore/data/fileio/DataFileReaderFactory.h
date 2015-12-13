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

#ifndef _DATA_FILE_READER_FACTORY_H_
#define _DATA_FILE_READER_FACTORY_H_

#include <QString>

#include "CSVFormat.h"
#include "MIDIFileReader.h"

class DataFileReader;
class Model;

class DataFileReaderFactory
{
public:
    enum Exception { ImportCancelled };

    /**
     * Return the file extensions that we have data file readers for,
     * in a format suitable for use with QFileDialog.  For example,
     * "*.csv *.xml".
     */
    static QString getKnownExtensions();

    /**
     * Return a data file reader initialised to the file at the
     * given path, or NULL if no suitable reader for this path is
     * available or the file cannot be opened.
     *
     * Caller owns the returned object and must delete it after use.
     * 
     * Note that this function is non-interactive -- the user is not
     * asked for file format preferences.
     */
    static DataFileReader *createReader(QString path,
                                        MIDIFileImportPreferenceAcquirer *,
					sv_samplerate_t mainModelSampleRate);

    /**
     * Read the given path, if a suitable reader is available.
     * Return NULL if no reader succeeded in reading this file.
     * 
     * Note that this function is non-interactive -- the user is not
     * asked for file format preferences.  If the CSV file reader is
     * used, it is with default format.
     */
    static Model *load(QString path,
                       MIDIFileImportPreferenceAcquirer *acquirer,
                       sv_samplerate_t mainModelSampleRate);

    /**
     * Read the given path, if a suitable reader is available.
     * Return NULL if no reader succeeded in reading this file.
     * Do not attempt the general CSV reader.
     */
    static Model *loadNonCSV(QString path,
                             MIDIFileImportPreferenceAcquirer *acquirer,
                             sv_samplerate_t mainModelSampleRate);

    /**
     * Read the given path using the CSV reader with the given format.
     * Return NULL if it failed in reading this file.
     */
    static Model *loadCSV(QString path,
                          CSVFormat format,
                          sv_samplerate_t mainModelSampleRate);

protected:
    static DataFileReader *createReader(QString path, bool csv,
                                        MIDIFileImportPreferenceAcquirer *,
                                        CSVFormat format,
					sv_samplerate_t mainModelSampleRate);
};

#endif

