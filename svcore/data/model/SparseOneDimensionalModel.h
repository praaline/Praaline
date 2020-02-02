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

#ifndef _SPARSE_ONE_DIMENSIONAL_MODEL_H_
#define _SPARSE_ONE_DIMENSIONAL_MODEL_H_

#include "SparseModel.h"
#include "NoteData.h"
#include "base/PlayParameterRepository.h"
#include "PraalineCore/Base/RealTime.h"

#include <QStringList>

struct OneDimensionalPoint
{
public:
    OneDimensionalPoint(sv_frame_t _frame) : frame(_frame) { }
    OneDimensionalPoint(sv_frame_t _frame, QString _label) : frame(_frame), label(_label) { }

    int getDimensions() const { return 1; }
    
    sv_frame_t frame;
    QString label;

    QString getLabel() const { return label; }

    void toXml(QTextStream &stream,
               QString indent = "",
               QString extraAttributes = "") const
    {
        stream << QString("%1<point frame=\"%2\" label=\"%3\" %4/>\n")
                  .arg(indent).arg(frame).arg(XmlExportable::encodeEntities(label))
                  .arg(extraAttributes);
    }

    QString toDelimitedDataString(QString delimiter, DataExportOptions, sv_samplerate_t sampleRate) const
    {
        QStringList list;
        list << RealTime::frame2RealTime(frame, sampleRate).toString().c_str();
        if (label != "") list << label;
        return list.join(delimiter);
    }

    struct Comparator {
        bool operator()(const OneDimensionalPoint &p1,
                        const OneDimensionalPoint &p2) const {
            if (p1.frame != p2.frame) return p1.frame < p2.frame;
            return p1.label < p2.label;
        }
    };
    
    struct OrderComparator {
        bool operator()(const OneDimensionalPoint &p1,
                        const OneDimensionalPoint &p2) const {
            return p1.frame < p2.frame;
        }
    };
};


class SparseOneDimensionalModel : public SparseModel<OneDimensionalPoint>,
        public NoteExportable
{
    Q_OBJECT
    
public:
    SparseOneDimensionalModel(sv_samplerate_t sampleRate, int resolution,
                              bool notifyOnAdd = true) :
        SparseModel<OneDimensionalPoint>(sampleRate, resolution, notifyOnAdd)
    {
        PlayParameterRepository::getInstance()->addPlayable(this);
    }

    virtual ~SparseOneDimensionalModel()
    {
        PlayParameterRepository::getInstance()->removePlayable(this);
    }

    virtual bool canPlay() const { return true; }

    virtual QString getDefaultPlayClipId() const
    {
        return "tap";
    }

    int getIndexOf(const Point &point)
    {
        // slow
        int i = 0;
        Point::Comparator comparator;
        for (PointList::const_iterator j = m_points.begin();
             j != m_points.end(); ++j, ++i) {
            if (!comparator(*j, point) && !comparator(point, *j)) return i;
        }
        return -1;
    }

    QString getTypeName() const { return tr("Sparse 1-D"); }
    std::string getType() const { return "SparseOneDimensionalModel"; }

    /**
     * TabularModel methods.
     */
    
    virtual int getColumnCount() const
    {
        return 3;
    }

    virtual QString getHeading(int column) const
    {
        switch (column) {
        case 0: return tr("Time");
        case 1: return tr("Frame");
        case 2: return tr("Label");
        default: return tr("Unknown");
        }
    }

    virtual QVariant getData(int row, int column, int role) const
    {
        if (column < 2) {
            return SparseModel<OneDimensionalPoint>::getData
                    (row, column, role);
        }

        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return QVariant();

        switch (column) {
        case 2: return i->label;
        default: return QVariant();
        }
    }

    virtual UndoableCommand *getSetDataCommand(int row, int column, const QVariant &value, int role)
    {
        if (column < 2) {
            return SparseModel<OneDimensionalPoint>::getSetDataCommand
                    (row, column, value, role);
        }

        if (role != Qt::EditRole) return 0;
        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return 0;
        EditCommand *command = new EditCommand(this, tr("Edit Data"));

        Point point(*i);
        command->deletePoint(point);

        switch (column) {
        case 2: point.label = value.toString(); break;
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
        if (column == 2) return SortAlphabetical;
        return SortNumeric;
    }

    /**
     * NoteExportable methods.
     */

    NoteList getNotes() const {
        return getNotesWithin(getStartFrame(), getEndFrame());
    }

    NoteList getNotesWithin(sv_frame_t startFrame, sv_frame_t endFrame) const {
        
        PointList points = getPoints(startFrame, endFrame);
        NoteList notes;

        for (PointList::iterator pli =
             points.begin(); pli != points.end(); ++pli) {

            notes.push_back
                    (NoteData(pli->frame,
                              sv_frame_t(getSampleRate() / 6), // arbitrary short duration
                              64,   // default pitch
                              100)); // default velocity
        }

        return notes;
    }
};

#endif



