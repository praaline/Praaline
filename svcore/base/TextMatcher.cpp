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

#include "TextMatcher.h"

TextMatcher::TextMatcher()
{
}

TextMatcher::~TextMatcher()
{
}

void
TextMatcher::test(Match &match, QStringList keywords, QString text,
		  QString textType, int score)
{
/*
    if (text.toLower() == keyword.toLower()) {
        match.score += score * 1.5;
        match.fragments << tr("%1: <b>%2</b>").arg(textType).arg(text);
        return;
    }
*/
    int len = text.length();
    int prevEnd = 0;
    QString fragment;

    while (1) {

        bool first = (prevEnd == 0);
        
        int idx = -1;
        QString keyword;

        for (int ki = 0; ki < keywords.size(); ++ki) {
            int midx = text.indexOf(keywords[ki], prevEnd, Qt::CaseInsensitive);
            if (midx >= 0 && midx < len) {
                if (midx < idx || idx == -1) {
                    idx = midx;
                    keyword = keywords[ki];
                }
            }
        }

        if (idx < 0 || idx >= len) break;

        int klen = keyword.length();

        if (first) {
            match.score += score;
        } else {
            match.score += score / 4;
        }

        int start = idx;
        int end = start + klen;

        if (start == 0) match.score += 1;
        if (end == len) match.score += 1;

        if (start > prevEnd + 14) {
            QString s = text.right((len - start) + 10);
            s = XmlExportable::encodeEntities(s.left(10)) + "<b>" +
                XmlExportable::encodeEntities(s.left(klen + 10).right(klen))
                + "</b>";
            fragment += QString("...%1").arg(s);
        } else {
            QString s = text.right(len - prevEnd);
            s = XmlExportable::encodeEntities(s.left(start - prevEnd)) + "<b>" +
                XmlExportable::encodeEntities(s.left(end - prevEnd).right(klen))
                + "</b>";
            fragment += s;
        }

        prevEnd = end;
    }

    if (prevEnd > 0 && prevEnd < len) {
        int n = len - prevEnd;
        fragment +=
            XmlExportable::encodeEntities(text.right(n).left(n < 8 ? n : 8));
    }

    if (fragment != "") {
        match.fragments[textType] = fragment;
    }
}

bool
TextMatcher::Match::operator<(const Match &m) const
{
    if (score != m.score) {
        return score < m.score;
    }
    if (key != m.key) {
        return key < m.key;
    }
    if (fragments.size() != m.fragments.size()) {
        return fragments.size() < m.fragments.size();
    }

    for (FragmentMap::const_iterator
             i = fragments.begin(),
             j = m.fragments.begin();
         i != fragments.end(); ++i, ++j) {
        if (i->first != j->first) return i->first < j->first;
        if (i->second != j->second) return i->second < j->second;
    }

    return false;
}
