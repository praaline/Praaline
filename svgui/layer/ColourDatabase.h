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

#ifndef _COLOUR_DATABASE_H_
#define _COLOUR_DATABASE_H_

#include <QObject>
#include <QString>
#include <QColor>
#include <QSize>
#include <QPixmap>
#include <vector>

class ColourDatabase : public QObject
{
    Q_OBJECT

public:
    static ColourDatabase *getInstance();

    int getColourCount() const;
    QString getColourName(int c) const;
    QColor getColour(int c) const;
    QColor getColour(QString name) const;
    int getColourIndex(QString name) const; // -1 -> not found
    int getColourIndex(QColor c) const; // returns first index of possibly many
    bool haveColour(QColor c) const;

    bool useDarkBackground(int c) const;
    void setUseDarkBackground(int c, bool dark);

    int addColour(QColor, QString); // returns index
    void removeColour(QString);

    // returned colour is not necessarily in database
    QColor getContrastingColour(int c) const;

    // for use in XML export
    void getStringValues(int index,
                         QString &colourName,
                         QString &colourSpec,
                         QString &darkbg) const;

    // for use in XML import
    int putStringValues(QString colourName,
                        QString colourSpec,
                        QString darkbg);

    // for use by PropertyContainer getPropertyRangeAndValue methods
    void getColourPropertyRange(int *min, int *max) const;

    QPixmap getExamplePixmap(int index, QSize size) const;
    
signals:
    void colourDatabaseChanged();

protected:
    ColourDatabase();

    struct ColourRec {
        QColor colour;
        QString name;
        bool darkbg;
    };
    
    typedef std::vector<ColourRec> ColourList;
    ColourList m_colours;

    static ColourDatabase m_instance;
};

#endif
