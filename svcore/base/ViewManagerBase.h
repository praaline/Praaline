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

#ifndef _VIEW_MANAGER_BASE_H_
#define _VIEW_MANAGER_BASE_H_

#include <QObject>

#include "Selection.h"

class AudioPlaySource;

/**
 * Base class for ViewManager, with no GUI content.  This should
 * define all of the ViewManager interface that e.g. audio I/O classes
 * need to refer to.
 */

class ViewManagerBase : public QObject
{
    Q_OBJECT

public:
    virtual ~ViewManagerBase();

    virtual void setAudioPlaySource(AudioPlaySource *source) = 0;

    virtual sv_frame_t alignPlaybackFrameToReference(sv_frame_t) const = 0;
    virtual sv_frame_t alignReferenceToPlaybackFrame(sv_frame_t) const = 0;

    virtual const MultiSelection &getSelection() const = 0;
    virtual const MultiSelection::SelectionList &getSelections() const = 0;
    virtual sv_frame_t constrainFrameToSelection(sv_frame_t frame) const = 0;

    virtual Selection getContainingSelection
    (sv_frame_t frame, bool defaultToFollowing) const = 0;

    virtual bool getPlayLoopMode() const = 0;
    virtual bool getPlaySelectionMode() const = 0;
    virtual bool getPlaySoloMode() const = 0;
    virtual bool getAlignMode() const = 0;

signals:
    /** Emitted when the selection has changed. */
    void selectionChanged();

    /** Emitted when the play loop mode has been changed. */
    void playLoopModeChanged();
    void playLoopModeChanged(bool);

    /** Emitted when the play selection mode has been changed. */
    void playSelectionModeChanged();
    void playSelectionModeChanged(bool);

    /** Emitted when the play solo mode has been changed. */
    void playSoloModeChanged();
    void playSoloModeChanged(bool);

    /** Emitted when the alignment mode has been changed. */
    void alignModeChanged();
    void alignModeChanged(bool);
};

#endif

