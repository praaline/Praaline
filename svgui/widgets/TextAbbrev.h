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

#ifndef _TEXT_ABBREV_H_
#define _TEXT_ABBREV_H_

#include <QString>
#include <QStringList>

class QFontMetrics;

class TextAbbrev 
{
public:
    enum Policy {
        ElideEnd,
        ElideEndAndCommonPrefixes,
        ElideStart,
        ElideMiddle
    };

    /**
     * Abbreviate the given text to the given maximum length
     * (including ellipsis), using the given abbreviation policy.  If
     * fuzzy is true, the text will be left alone if it is "not much
     * more than" the maximum length.
     * 
     * If ellipsis is non-empty, it will be used to show elisions in
     * preference to the default (which is "...").
     */
    static QString abbreviate(QString text, int maxLength,
                              Policy policy = ElideEnd,
                              bool fuzzy = true,
                              QString ellipsis = "");

    /**
     * Abbreviate the given text to the given maximum painted width,
     * using the given abbreviation policy.  maxWidth is also modified
     * so as to return the painted width of the abbreviated text.
     *
     * If ellipsis is non-empty, it will be used to show elisions in
     * preference to the default (which is tr("...")).
     */
    static QString abbreviate(QString text,
                              const QFontMetrics &metrics,
                              int &maxWidth,
                              Policy policy = ElideEnd,
                              QString ellipsis = "");
    
    /**
     * Abbreviate all of the given texts to the given maximum length,
     * using the given abbreviation policy.  If fuzzy is true, texts
     * that are "not much more than" the maximum length will be left
     * alone.
     *
     * If ellipsis is non-empty, it will be used to show elisions in
     * preference to the default (which is tr("...")).
     */
    static QStringList abbreviate(const QStringList &texts, int maxLength,
                                  Policy policy = ElideEndAndCommonPrefixes,
                                  bool fuzzy = true,
                                  QString ellipsis = "");

    /**
     * Abbreviate all of the given texts to the given maximum painted
     * width, using the given abbreviation policy.  maxWidth is also
     * modified so as to return the maximum painted width of the
     * abbreviated texts.
     *
     * If ellipsis is non-empty, it will be used to show elisions in
     * preference to the default (which is tr("...")).
     */
    static QStringList abbreviate(const QStringList &texts,
                                  const QFontMetrics &metrics,
                                  int &maxWidth,
                                  Policy policy = ElideEndAndCommonPrefixes,
                                  QString ellipsis = "");

protected:
    static QString getDefaultEllipsis();
    static int getFuzzLength(QString ellipsis);
    static int getFuzzWidth(const QFontMetrics &metrics, QString ellipsis);
    static QString abbreviateTo(QString text, int characters,
                                Policy policy, QString ellipsis);
    static QStringList elidePrefixes(const QStringList &texts,
                                     int targetReduction,
                                     QString ellipsis);
    static QStringList elidePrefixes(const QStringList &texts,
                                     const QFontMetrics &metrics,
                                     int targetWidthReduction,
                                     QString ellipsis);
    static int getPrefixLength(const QStringList &texts);
};

#endif

