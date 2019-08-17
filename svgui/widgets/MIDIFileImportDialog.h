/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _MIDI_FILE_IMPORT_DIALOG_H_
#define _MIDI_FILE_IMPORT_DIALOG_H_

#include <QObject>

#include "data/fileio/MIDIFileReader.h"

class MIDIFileImportDialog : public QObject,
                             public MIDIFileImportPreferenceAcquirer
{
    Q_OBJECT

public:
    MIDIFileImportDialog(QWidget *parent = nullptr);

    virtual TrackPreference getTrackImportPreference
    (QStringList trackNames, bool haveSomePercussion,
     QString &singleTrack) const;

    virtual void showError(QString error);

protected:
    QWidget *m_parent;
};

#endif
