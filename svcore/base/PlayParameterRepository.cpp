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

#include "PlayParameterRepository.h"
#include "PlayParameters.h"
#include "Playable.h"

#include <iostream>

PlayParameterRepository *
PlayParameterRepository::m_instance = new PlayParameterRepository;

PlayParameterRepository *
PlayParameterRepository::getInstance()
{
    return m_instance;
}

PlayParameterRepository::~PlayParameterRepository()
{
}

void
PlayParameterRepository::addPlayable(const Playable *playable)
{
    cerr << "PlayParameterRepository:addPlayable playable = " << playable <<  endl;

    if (!getPlayParameters(playable)) {

	// Give all playables the same type of play parameters for the
	// moment

        cerr << "PlayParameterRepository:addPlayable: Adding play parameters for " << playable << endl;

        PlayParameters *params = new PlayParameters;
        m_playParameters[playable] = params;

        params->setPlayClipId
            (playable->getDefaultPlayClipId());

        params->setPlayAudible
            (playable->getDefaultPlayAudible());
        
        connect(params, SIGNAL(playParametersChanged()),
                this, SLOT(playParametersChanged()));
        
        connect(params, SIGNAL(playClipIdChanged(QString)),
                this, SLOT(playClipIdChanged(QString)));

        cerr << "Connected play parameters " << params << " for playable "
                     << playable << " to this " << this << endl;
    }
}    

void
PlayParameterRepository::removePlayable(const Playable *playable)
{
    if (m_playParameters.find(playable) == m_playParameters.end()) {
        cerr << "WARNING: PlayParameterRepository::removePlayable: unknown playable " << playable << endl;
        return;
    }
    delete m_playParameters[playable];
    m_playParameters.erase(playable);
}

void
PlayParameterRepository::copyParameters(const Playable *from, const Playable *to)
{
    if (!getPlayParameters(from)) {
        cerr << "ERROR: PlayParameterRepository::copyParameters: source playable unknown" << endl;
        return;
    }
    if (!getPlayParameters(to)) {
        cerr << "WARNING: PlayParameterRepository::copyParameters: target playable unknown, adding it now" << endl;
        addPlayable(to);
    }
    getPlayParameters(to)->copyFrom(getPlayParameters(from));
}

PlayParameters *
PlayParameterRepository::getPlayParameters(const Playable *playable) 
{
    if (m_playParameters.find(playable) == m_playParameters.end()) return 0;
    return m_playParameters.find(playable)->second;
}

void
PlayParameterRepository::playParametersChanged()
{
    PlayParameters *params = dynamic_cast<PlayParameters *>(sender());
    emit playParametersChanged(params);
}

void
PlayParameterRepository::playClipIdChanged(QString id)
{
    PlayParameters *params = dynamic_cast<PlayParameters *>(sender());
    for (PlayableParameterMap::iterator i = m_playParameters.begin();
         i != m_playParameters.end(); ++i) {
        if (i->second == params) {
            emit playClipIdChanged(i->first, id);
            return;
        }
    }
}

void
PlayParameterRepository::clear()
{
//    cerr << "PlayParameterRepository: PlayParameterRepository::clear" << endl;
    while (!m_playParameters.empty()) {
	delete m_playParameters.begin()->second;
	m_playParameters.erase(m_playParameters.begin());
    }
}

PlayParameterRepository::EditCommand::EditCommand(PlayParameters *params) :
    m_params(params)
{
    m_from.copyFrom(m_params);
    m_to.copyFrom(m_params);
}

void
PlayParameterRepository::EditCommand::setPlayMuted(bool muted)
{
    m_to.setPlayMuted(muted);
}

void
PlayParameterRepository::EditCommand::setPlayAudible(bool audible)
{
    m_to.setPlayAudible(audible);
}

void
PlayParameterRepository::EditCommand::setPlayPan(float pan)
{
    m_to.setPlayPan(pan);
}

void
PlayParameterRepository::EditCommand::setPlayGain(float gain)
{
    m_to.setPlayGain(gain);
}

void
PlayParameterRepository::EditCommand::setPlayClipId(QString id)
{
    m_to.setPlayClipId(id);
}

void
PlayParameterRepository::EditCommand::execute()
{
    m_params->copyFrom(&m_to);
}

void
PlayParameterRepository::EditCommand::unexecute()
{
    m_params->copyFrom(&m_from);
}
    
QString
PlayParameterRepository::EditCommand::getName() const
{
    QString name;
    QString multiname = tr("Adjust Playback Parameters");

    int changed = 0;

    if (m_to.isPlayAudible() != m_from.isPlayAudible()) {
        name = tr("Change Playback Mute State");
        if (++changed > 1) return multiname;
    }

    if (m_to.getPlayGain() != m_from.getPlayGain()) {
        name = tr("Change Playback Gain");
        if (++changed > 1) return multiname;
    }

    if (m_to.getPlayPan() != m_from.getPlayPan()) {
        name = tr("Change Playback Pan");
        if (++changed > 1) return multiname;
    }

    if (m_to.getPlayClipId() != m_from.getPlayClipId()) {
        name = tr("Change Playback Sample");
        if (++changed > 1) return multiname;
    }

    if (name == "") return multiname;
    return name;
}

