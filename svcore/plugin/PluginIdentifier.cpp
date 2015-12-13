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

/*
   This is a modified version of a source file from the 
   Rosegarden MIDI and audio sequencer and notation editor.
   This file copyright 2000-2006 Chris Cannam and QMUL.
*/

#include "PluginIdentifier.h"
#include <iostream>
#include <QFileInfo>

QString
PluginIdentifier::createIdentifier(QString type,
				   QString soName,
				   QString label)
{
    QString identifier = type + ":" + QFileInfo(soName).baseName() + ":" + label;
    return identifier;
}

QString
PluginIdentifier::canonicalise(QString identifier)
{
    QString type, soName, label;
    parseIdentifier(identifier, type, soName, label);
    return createIdentifier(type, soName, label);
}

void
PluginIdentifier::parseIdentifier(QString identifier,
				  QString &type,
				  QString &soName,
				  QString &label)
{
    type = identifier.section(':', 0, 0);
    soName = identifier.section(':', 1, 1);
    label = identifier.section(':', 2);
}

bool
PluginIdentifier::areIdentifiersSimilar(QString id1, QString id2)
{
    QString type1, type2, soName1, soName2, label1, label2;

    parseIdentifier(id1, type1, soName1, label1);
    parseIdentifier(id2, type2, soName2, label2);

    if (type1 != type2 || label1 != label2) return false;

    bool similar = (soName1.section('/', -1).section('.', 0, 0) ==
		    soName2.section('/', -1).section('.', 0, 0));

    return similar;
}

QString
PluginIdentifier::BUILTIN_PLUGIN_SONAME = "_builtin";

QString
PluginIdentifier::RESERVED_PROJECT_DIRECTORY_KEY = "__QMUL__:__RESERVED__:ProjectDirectoryKey";

