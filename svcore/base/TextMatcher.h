/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
   
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _TEXT_MATCHER_H_
#define _TEXT_MATCHER_H_

#include <QString>
#include <QStringList>
#include "XmlExportable.h"

#include <map>

/// A rather eccentric interface for matching texts in differently-scored fields

class TextMatcher
{
public:
    TextMatcher();
    virtual ~TextMatcher();
    
    struct Match
    {
        QString key; // This field is not used by TextMatcher
        int score;
        typedef std::map<QString, QString> FragmentMap; // text type -> fragment
        FragmentMap fragments;

        Match() : score(0) { }
        Match(const Match &m) :
            key(m.key), score(m.score), fragments(m.fragments) { }

        bool operator<(const Match &m) const; // sort by score first
    };

    void test(Match &match, // existing match record to be augmented
              QStringList keywords, // search terms
              QString text, // to search within
              QString textType, // key to use for fragment map
              int score); // relative weight for hits within this text type

};


#endif
