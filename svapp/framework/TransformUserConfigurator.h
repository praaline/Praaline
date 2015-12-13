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

#ifndef _TRANSFORM_USER_CONFIGURATOR_H_
#define _TRANSFORM_USER_CONFIGURATOR_H_

#include "transform/ModelTransformerFactory.h"

class TransformUserConfigurator : public ModelTransformerFactory::UserConfigurator
{
public:
    // This is of course absolutely gross

    virtual bool configure(ModelTransformer::Input &input,
                           Transform &transform,
                           Vamp::PluginBase *plugin,
                           Model *&inputModel,
			   AudioPlaySource *source,
			   sv_frame_t startFrame,
			   sv_frame_t duration,
			   const QMap<QString, Model *> &modelMap,
                           QStringList candidateModelNames,
                           QString defaultModelName);

    static void setParentWidget(QWidget *);

private:
    bool getChannelRange(TransformId identifier,
                         Vamp::PluginBase *plugin, int &min, int &max);

};

#endif
