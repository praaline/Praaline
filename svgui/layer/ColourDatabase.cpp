/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "base/BaseTypes.h"
#include "ColourDatabase.h"
#include "base/XmlExportable.h"

#include <QPainter>

ColourDatabase
ColourDatabase::m_instance;

ColourDatabase *
ColourDatabase::getInstance()
{
    return &m_instance;
}

ColourDatabase::ColourDatabase()
{
}

int
ColourDatabase::getColourCount() const
{
    return int(m_colours.size());
}

QString
ColourDatabase::getColourName(int c) const
{
    if (!in_range_for(m_colours, c)) return "";
    return m_colours[c].name;
}

QColor
ColourDatabase::getColour(int c) const
{
    if (!in_range_for(m_colours, c)) return Qt::black;
    return m_colours[c].colour;
}

QColor
ColourDatabase::getColour(QString name) const
{
    for (auto &c: m_colours) {
        if (c.name == name) return c.colour;
    }

    return Qt::black;
}

int
ColourDatabase::getColourIndex(QString name) const
{
    int index = 0;
    for (auto &c: m_colours) {
        if (c.name == name) return index;
        ++index;
    }

    return -1;
}

int
ColourDatabase::getColourIndex(QColor col) const
{
    int index = 0;
    for (auto &c: m_colours) {
        if (c.colour == col) return index;
        ++index;
    }

    return -1;
}

bool
ColourDatabase::useDarkBackground(int c) const
{
    if (!in_range_for(m_colours, c)) return false;
    return m_colours[c].darkbg;
}

void
ColourDatabase::setUseDarkBackground(int c, bool dark)
{
    if (!in_range_for(m_colours, c)) return;
    if (m_colours[c].darkbg != dark) {
        m_colours[c].darkbg = dark;
        emit colourDatabaseChanged();
    }
}

int
ColourDatabase::addColour(QColor c, QString name)
{
    int index = 0;

    for (ColourList::iterator i = m_colours.begin();
         i != m_colours.end(); ++i) {
        if (i->name == name) {
            i->colour = c;
            return index;
        }
        ++index;
    }

    ColourRec rec;
    rec.colour = c;
    rec.name = name;
    rec.darkbg = false;
    m_colours.push_back(rec);
    emit colourDatabaseChanged();
    return index;
}

void
ColourDatabase::removeColour(QString name)
{
    for (ColourList::iterator i = m_colours.begin();
         i != m_colours.end(); ++i) {
        if (i->name == name) {
            m_colours.erase(i);
            return;
        }
    }
}

void
ColourDatabase::getStringValues(int index,
                                QString &colourName,
                                QString &colourSpec,
                                QString &darkbg) const
{
    colourName = "";
    colourSpec = "";
    if (!in_range_for(m_colours, index)) return;

    colourName = getColourName(index);
    QColor c = getColour(index);
    colourSpec = XmlExportable::encodeColour(c.red(), c.green(), c.blue());
    darkbg = useDarkBackground(index) ? "true" : "false";
}

int
ColourDatabase::putStringValues(QString colourName,
                                QString colourSpec,
                                QString darkbg)
{
    int index = -1;
    if (colourSpec != "") {
	QColor colour(colourSpec);
        index = getColourIndex(colour);
        if (index < 0) {
            index = addColour(colour,
                              colourName == "" ? colourSpec : colourName);
        }
    } else if (colourName != "") {
        index = getColourIndex(colourName);
    }
    if (index >= 0) {
        setUseDarkBackground(index, darkbg == "true");
    }
    return index;
}

void
ColourDatabase::getColourPropertyRange(int *min, int *max) const
{
    ColourDatabase *db = getInstance();
    if (min) *min = 0;
    if (max) {
        *max = 0;
        if (db->getColourCount() > 0) *max = db->getColourCount()-1;
    }
}

QPixmap
ColourDatabase::getExamplePixmap(int index, QSize size) const
{
    QPixmap pmap(size);
    pmap.fill(useDarkBackground(index) ? Qt::black : Qt::white);
    QPainter paint(&pmap);
    QColor colour(getColour(index));
    paint.setPen(colour);
    paint.setBrush(colour);
    int margin = 2;
    if (size.width() < 4 || size.height() < 4) margin = 0;
    else if (size.width() < 8 || size.height() < 8) margin = 1;
    paint.drawRect(margin, margin,
                   size.width() - margin*2 - 1, size.height() - margin*2 - 1);
    return pmap;
}

