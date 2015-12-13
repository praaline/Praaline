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

#include "TextAbbrev.h"

#include <QFontMetrics>
#include <QApplication>

#include <iostream>

QString
TextAbbrev::getDefaultEllipsis()
{
    return "...";
}

int
TextAbbrev::getFuzzLength(QString ellipsis)
{
    int len = ellipsis.length();
    if (len < 3) return len + 3;
    else if (len > 5) return len + 5;
    else return len * 2;
}

int
TextAbbrev::getFuzzWidth(const QFontMetrics &metrics, QString ellipsis)
{
    int width = metrics.width(ellipsis);
    return width * 2;
}

QString
TextAbbrev::abbreviateTo(QString text, int characters, Policy policy,
                         QString ellipsis)
{
    switch (policy) {

    case ElideEnd:
    case ElideEndAndCommonPrefixes:
        text = text.left(characters) + ellipsis;
        break;
        
    case ElideStart:
        text = ellipsis + text.right(characters);
        break;

    case ElideMiddle:
        if (characters > 2) {
            text = text.left(characters/2 + 1) + ellipsis
                + text.right(characters - (characters/2 + 1));
        } else {
            text = text.left(characters) + ellipsis;
        }
        break;
    }

    return text;
}

QString
TextAbbrev::abbreviate(QString text, int maxLength, Policy policy, bool fuzzy,
                       QString ellipsis)
{
    if (ellipsis == "") ellipsis = getDefaultEllipsis();
    int fl = (fuzzy ? getFuzzLength(ellipsis) : 0);
    if (maxLength <= ellipsis.length()) maxLength = ellipsis.length() + 1;
    if (text.length() <= maxLength + fl) return text;

    int truncated = maxLength - ellipsis.length();
    return abbreviateTo(text, truncated, policy, ellipsis);
}

QString
TextAbbrev::abbreviate(QString text,
                       const QFontMetrics &metrics, int &maxWidth,
                       Policy policy, QString ellipsis)
{
    if (ellipsis == "") ellipsis = getDefaultEllipsis();

    int tw = metrics.width(text);

    if (tw <= maxWidth) {
        maxWidth = tw;
        return text;
    }

    int truncated = text.length();
    QString original = text;

    while (tw > maxWidth && truncated > 1) {

        truncated--;

        if (truncated > ellipsis.length()) {
            text = abbreviateTo(original, truncated, policy, ellipsis);
        } else {
            break;
        }

        tw = metrics.width(text);
    }

    maxWidth = tw;
    return text;
}

QStringList
TextAbbrev::abbreviate(const QStringList &texts, int maxLength,
                       Policy policy, bool fuzzy, QString ellipsis)
{
    if (policy == ElideEndAndCommonPrefixes &&
        texts.size() > 1) {

        if (ellipsis == "") ellipsis = getDefaultEllipsis();
        int fl = (fuzzy ? getFuzzLength(ellipsis) : 0);
        if (maxLength <= ellipsis.length()) maxLength = ellipsis.length() + 1;

        int maxOrigLength = 0;
        for (int i = 0; i < texts.size(); ++i) {
            int len = texts[i].length();
            if (len > maxOrigLength) maxOrigLength = len;
        }
        if (maxOrigLength <= maxLength + fl) return texts;

        return abbreviate(elidePrefixes
                          (texts, maxOrigLength - maxLength, ellipsis),
                          maxLength, ElideEnd, fuzzy, ellipsis);
    }

    QStringList results;
    for (int i = 0; i < texts.size(); ++i) {
        results.push_back
            (abbreviate(texts[i], maxLength, policy, fuzzy, ellipsis));
    }
    return results;
}

QStringList
TextAbbrev::abbreviate(const QStringList &texts, const QFontMetrics &metrics,
                       int &maxWidth, Policy policy, QString ellipsis)
{
    if (policy == ElideEndAndCommonPrefixes &&
        texts.size() > 1) {

        if (ellipsis == "") ellipsis = getDefaultEllipsis();

        int maxOrigWidth = 0;
        for (int i = 0; i < texts.size(); ++i) {
            int w = metrics.width(texts[i]);
            if (w > maxOrigWidth) maxOrigWidth = w;
        }

        return abbreviate(elidePrefixes(texts, metrics,
                                        maxOrigWidth - maxWidth, ellipsis),
                          metrics, maxWidth, ElideEnd, ellipsis);
    }

    QStringList results;
    int maxAbbrWidth = 0;
    for (int i = 0; i < texts.size(); ++i) {
        int width = maxWidth;
        QString abbr = abbreviate(texts[i], metrics, width, policy, ellipsis);
        if (width > maxAbbrWidth) maxAbbrWidth = width;
        results.push_back(abbr);
    }
    maxWidth = maxAbbrWidth;
    return results;
}

QStringList
TextAbbrev::elidePrefixes(const QStringList &texts,
                          int targetReduction,
                          QString ellipsis)
{
    if (texts.empty()) return texts;
    int plen = getPrefixLength(texts);
    int fl = getFuzzLength(ellipsis);
    if (plen < fl) return texts;

    QString prefix = texts[0].left(plen);
    int truncated = plen;
    if (plen >= targetReduction + fl) {
        truncated = plen - targetReduction;
    } else {
        truncated = fl;
    }
    prefix = abbreviate(prefix, truncated, ElideEnd, false, ellipsis);

    QStringList results;
    for (int i = 0; i < texts.size(); ++i) {
        results.push_back
            (prefix + texts[i].right(texts[i].length() - plen));
    }
    return results;
}

QStringList
TextAbbrev::elidePrefixes(const QStringList &texts,
                          const QFontMetrics &metrics,
                          int targetWidthReduction,
                          QString ellipsis)
{
    if (texts.empty()) return texts;
    int plen = getPrefixLength(texts);
    int fl = getFuzzLength(ellipsis);
    if (plen < fl) return texts;

    QString prefix = texts[0].left(plen);
    int pwid = metrics.width(prefix);
    int twid = pwid - targetWidthReduction;
    if (twid < metrics.width(ellipsis) * 2) twid = metrics.width(ellipsis) * 2;
    prefix = abbreviate(prefix, metrics, twid, ElideEnd, ellipsis);

    QStringList results;
    for (int i = 0; i < texts.size(); ++i) {
        results.push_back
            (prefix + texts[i].right(texts[i].length() - plen));
    }
    return results;
}

static bool
havePrefix(QString prefix, const QStringList &texts)
{
    for (int i = 1; i < texts.size(); ++i) {
        if (!texts[i].startsWith(prefix)) return false;
    }
    return true;
}

int
TextAbbrev::getPrefixLength(const QStringList &texts)
{
    QString reference = texts[0];

    if (reference == "" || havePrefix(reference, texts)) {
        return reference.length();
    }

    int candidate = reference.length();
    QString splitChars(";:,./#-!()$_+=[]{}\\");

    while (--candidate > 1) {
        if (splitChars.contains(reference[candidate])) {
            if (havePrefix(reference.left(candidate), texts)) {
                break;
            }
        }
    }

//    cerr << "TextAbbrev::getPrefixLength: prefix length is " << candidate << endl;
//    for (int i = 0; i < texts.size(); ++i) {
//        cerr << texts[i].left(candidate) << "|" << texts[i].right(texts[i].length() - candidate) << endl;
//    }

    return candidate;
}

