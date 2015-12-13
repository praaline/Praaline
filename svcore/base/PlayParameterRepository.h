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

#ifndef _PLAY_PARAMETER_REPOSITORY_H_
#define _PLAY_PARAMETER_REPOSITORY_H_

#include "PlayParameters.h"
#include "Command.h"

class Playable;

#include <map>

#include <QObject>
#include <QString>

class PlayParameterRepository : public QObject
{
    Q_OBJECT

public:
    static PlayParameterRepository *getInstance();

    virtual ~PlayParameterRepository();

    void addPlayable(const Playable *playable);
    void removePlayable(const Playable *playable);
    void copyParameters(const Playable *from, const Playable *to);

    PlayParameters *getPlayParameters(const Playable *playable);

    void clear();

    class EditCommand : public SVCommand
    {
    public:
        EditCommand(PlayParameters *params);
        void setPlayMuted(bool);
        void setPlayAudible(bool);
        void setPlayPan(float);
        void setPlayGain(float);
        void setPlayClipId(QString);
        void execute();
        void unexecute();
        QString getName() const;

    protected:
        PlayParameters *m_params;
        PlayParameters m_from;
        PlayParameters m_to;
    };

signals:
    void playParametersChanged(PlayParameters *);
    void playClipIdChanged(const Playable *, QString);

protected slots:
    void playParametersChanged();
    void playClipIdChanged(QString);

protected:
    typedef std::map<const Playable *, PlayParameters *> PlayableParameterMap;
    PlayableParameterMap m_playParameters;

    static PlayParameterRepository *m_instance;
};

#endif
