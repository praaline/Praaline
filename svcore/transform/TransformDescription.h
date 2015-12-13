/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 Chris Cannam and QMUL.
   
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _TRANSFORM_DESCRIPTION_H_
#define _TRANSFORM_DESCRIPTION_H_

#include "Transform.h"

#include <QString>

#include <vector>

/**
 * Metadata associated with a transform.
 *
 * The transform ID is the same as that used in the Transform class.
 * It is intended to be computer-referenceable and unique within the
 * application.
 * 
 * The name is intended to be human readable.  In principle it doesn't
 * have to be unique, but the factory that creates these objects
 * should add suffixes to ensure that it is, all the same (just to
 * avoid user confusion).
 *
 * The friendly name is a shorter version of the name.
 *
 * The type is also intended to be user-readable, for use in menus.
 *
 * To obtain these objects, use
 * TransformFactory::getAllTransformDescriptions and
 * TransformFactory::getTransformDescription.
 */

struct TransformDescription
{
    enum Type {
        Analysis,    // e.g. vamp plugin output
        Effects,     // e.g. ladspa plugin with audio in and out
        EffectsData, // e.g. control output of ladspa plugin
        Generator,   // e.g. audio out of ladspa plugin with no audio in
        UnknownType
    };

    TransformDescription() : 
        type(UnknownType), configurable(false) { }
    TransformDescription(Type _type, QString _category,
                         TransformId _identifier, QString _name,
                         QString _friendlyName, QString _description,
                         QString _longDescription,
                         QString _maker, QString _units, bool _configurable) :
        type(_type), category(_category),
        identifier(_identifier), name(_name),
        friendlyName(_friendlyName), description(_description),
        longDescription(_longDescription),
        maker(_maker), units(_units), configurable(_configurable) { }

    Type type;
    QString category; // e.g. time > onsets
    TransformId identifier; // e.g. vamp:vamp-aubio:aubioonset
    QString name; // plugin's name if 1 output, else "name: output"
    QString friendlyName; // short text for layer name
    QString description; // sentence describing transform
    QString longDescription; // description "using" plugin name "by" maker
    QString maker;
    QString infoUrl;
    QString units;
    bool configurable;
    
    bool operator<(const TransformDescription &od) const {
        return
            (name <  od.name) ||
            (name == od.name && identifier < od.identifier);
    };
};

typedef std::vector<TransformDescription> TransformList;

#endif
