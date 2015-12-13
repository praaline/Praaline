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

#ifndef _PLUGIN_IDENTIFIER_H_
#define _PLUGIN_IDENTIFIER_H_

#include <QString>


// A plugin identifier is simply a string; this class provides methods
// to parse it into its constituent bits (plugin type, DLL path and label).

class PluginIdentifier {

public:
 
    static QString createIdentifier(QString type, QString soName, QString label);

    static QString canonicalise(QString identifier);

    static void parseIdentifier(QString identifier,
				QString &type, QString &soName, QString &label);

    static bool areIdentifiersSimilar(QString id1, QString id2);

    // Fake soName for use with plugins that are actually compiled in
    static QString BUILTIN_PLUGIN_SONAME;

    // Not strictly related to identifiers
    static QString RESERVED_PROJECT_DIRECTORY_KEY;
};

#endif
