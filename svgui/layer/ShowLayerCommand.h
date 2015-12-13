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

#ifndef _SHOW_LAYER_COMMAND_H_
#define _SHOW_LAYER_COMMAND_H_

#include "base/Command.h"

class ShowLayerCommand : public SVCommand
{
public:
    ShowLayerCommand(View *view, Layer *layer, bool show, QString commandName) :
        m_view(view), m_layer(layer), m_show(show), m_name(commandName) { }
    void execute() {
        m_layer->showLayer(m_view, m_show);
    }
    void unexecute() {
        m_layer->showLayer(m_view, !m_show);
    }
    QString getName() const {
        return m_name;
    }
protected:
    View *m_view;
    Layer *m_layer;
    bool m_show;
    QString m_name;
};

#endif
