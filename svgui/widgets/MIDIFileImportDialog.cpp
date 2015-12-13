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

#include "MIDIFileImportDialog.h"

#include <QMessageBox>
#include <QInputDialog>

MIDIFileImportDialog::MIDIFileImportDialog(QWidget *parent) :
    m_parent(parent)
{
}

MIDIFileImportDialog::TrackPreference
MIDIFileImportDialog::getTrackImportPreference(QStringList displayNames,
                                               bool haveSomePercussion,
                                               QString &singleTrack) const
{
    QStringList available;

    QString allTracks = tr("Merge all tracks");
    QString allNonPercussion = tr("Merge all non-percussion tracks");

    singleTrack = "";

    int nonTrackItems = 1;
    
    available << allTracks;

    if (haveSomePercussion) {
        available << allNonPercussion;
        ++nonTrackItems;
    }

    available << displayNames;

    bool ok = false;
    QString selected = QInputDialog::getItem
        (0, tr("Select track or tracks to import"),
         tr("<b>Select track to import</b><p>You can only import this file as a single annotation layer, but the file contains more than one track, or notes on more than one channel.<p>Please select the track or merged tracks you wish to import:"),
         available, 0, false, &ok);
    
    if (!ok || selected.isEmpty()) return ImportNothing;

    TrackPreference pref;
    if (selected == allTracks) pref = MergeAllTracks;
    else if (selected == allNonPercussion) pref = MergeAllNonPercussionTracks;
    else {
        singleTrack = selected;
        pref = ImportSingleTrack;
    }

    return pref;
}

void
MIDIFileImportDialog::showError(QString error)
{
    QMessageBox::critical(0, tr("Error in MIDI file import"), error);
}

