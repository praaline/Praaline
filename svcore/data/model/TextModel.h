/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _TEXT_MODEL_H_
#define _TEXT_MODEL_H_

#include "SparseModel.h"
#include "base/XmlExportable.h"
#include "base/RealTime.h"

#include <QStringList>

/**
 * Text point type for use in a SparseModel.  This represents a piece
 * of text at a given time and y-value in the [0,1) range (indicative
 * of height on the window).  Intended for casual textual annotations.
 */

struct TextPoint : public XmlExportable
{
public:
    TextPoint(sv_frame_t _frame) : frame(_frame), height(0.0f) { }
    TextPoint(sv_frame_t _frame, float _height, QString _label) :
        frame(_frame), height(_height), label(_label) { }

    int getDimensions() const { return 2; }
    
    sv_frame_t frame;
    float height;
    QString label;

    QString getLabel() const { return label; }
    
    void toXml(QTextStream &stream, QString indent = "",
               QString extraAttributes = "") const
    {
        stream << QString("%1<point frame=\"%2\" height=\"%3\" label=\"%4\" %5/>\n")
                  .arg(indent).arg(frame).arg(height)
                  .arg(encodeEntities(label)).arg(extraAttributes);
    }

    QString toDelimitedDataString(QString delimiter, DataExportOptions, sv_samplerate_t sampleRate) const
    {
        QStringList list;
        list << RealTime::frame2RealTime(frame, sampleRate).toString().c_str();
        list << QString("%1").arg(height);
        if (label != "") list << label;
        return list.join(delimiter);
    }

    struct Comparator {
        bool operator()(const TextPoint &p1,
                        const TextPoint &p2) const {
            if (p1.frame != p2.frame) return p1.frame < p2.frame;
            if (p1.height != p2.height) return p1.height < p2.height;
            return p1.label < p2.label;
        }
    };
    
    struct OrderComparator {
        bool operator()(const TextPoint &p1,
                        const TextPoint &p2) const {
            return p1.frame < p2.frame;
        }
    };
};


// Make this a class rather than a typedef so it can be predeclared.

class TextModel : public SparseModel<TextPoint>
{
    Q_OBJECT
    
public:
    TextModel(sv_samplerate_t sampleRate, int resolution, bool notifyOnAdd = true) :
        SparseModel<TextPoint>(sampleRate, resolution, notifyOnAdd)
    { }

    virtual void toXml(QTextStream &out,
                       QString indent = "",
                       QString extraAttributes = "") const
    {
        SparseModel<TextPoint>::toXml
                (out, indent,
                 QString("%1 subtype=\"text\"").arg(extraAttributes));
    }

    QString getTypeName() const { return tr("Text"); }
    std::string getType() const { return "TextModel"; }

    /**
     * TabularModel methods.
     */
    
    virtual int getColumnCount() const
    {
        return 4;
    }

    virtual QString getHeading(int column) const
    {
        switch (column) {
        case 0: return tr("Time");
        case 1: return tr("Frame");
        case 2: return tr("Height");
        case 3: return tr("Label");
        default: return tr("Unknown");
        }
    }

    virtual QVariant getData(int row, int column, int role) const
    {
        if (column < 2) {
            return SparseModel<TextPoint>::getData
                    (row, column, role);
        }

        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return QVariant();

        switch (column) {
        case 2: return i->height;
        case 3: return i->label;
        default: return QVariant();
        }
    }

    virtual UndoableCommand *getSetDataCommand(int row, int column, const QVariant &value, int role)
    {
        if (column < 2) {
            return SparseModel<TextPoint>::getSetDataCommand
                    (row, column, value, role);
        }

        if (role != Qt::EditRole) return 0;
        PointListIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return 0;
        EditCommand *command = new EditCommand(this, tr("Edit Data"));

        Point point(*i);
        command->deletePoint(point);

        switch (column) {
        case 2: point.height = float(value.toDouble()); break;
        case 3: point.label = value.toString(); break;
        }

        command->addPoint(point);
        return command->finish();
    }

    virtual bool isColumnTimeValue(int column) const
    {
        return (column < 2);
    }

    virtual SortType getSortType(int column) const
    {
        if (column == 3) return SortAlphabetical;
        return SortNumeric;
    }

};


#endif



