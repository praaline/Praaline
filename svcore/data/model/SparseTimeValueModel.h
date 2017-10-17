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

#ifndef _SPARSE_TIME_VALUE_MODEL_H_
#define _SPARSE_TIME_VALUE_MODEL_H_

#include "SparseValueModel.h"
#include "base/PlayParameterRepository.h"
#include "base/RealTime.h"

/**
 * Time/value point type for use in a SparseModel or SparseValueModel.
 * With this point type, the model basically represents a wiggly-line
 * plot with points at arbitrary intervals of the model resolution.
 */

struct TimeValuePoint
{
public:
    TimeValuePoint(long _frame) : frame(_frame), value(0.0f), confidenceInterval(0.0f) { }
    TimeValuePoint(long _frame, float _value, QString _label) :
        frame(_frame), value(_value), confidenceInterval(0.0f), label(_label) { }
    TimeValuePoint(long _frame, float _value, float _ci, QString _label) :
        frame(_frame), value(_value), confidenceInterval(_ci), label(_label) { }

    int getDimensions() const { return 2; }
    
    long frame;
    float value;
    float confidenceInterval;
    QString label;

    QString getLabel() const { return label; }
    
    void toXml(QTextStream &stream, QString indent = "",
               QString extraAttributes = "") const
    {
        stream << QString("%1<point frame=\"%2\" value=\"%3\" label=\"%4\" %5/>\n")
                  .arg(indent).arg(frame).arg(value).arg(XmlExportable::encodeEntities(label))
                  .arg(extraAttributes);
    }

    QString toDelimitedDataString(QString delimiter, DataExportOptions, sv_samplerate_t sampleRate) const
    {
        QStringList list;
        list << RealTime::frame2RealTime(frame, sampleRate).toString().c_str();
        list << QString("%1").arg(value);
        if (label != "") list << label;
        return list.join(delimiter);
    }

    struct Comparator {
        bool operator()(const TimeValuePoint &p1,
                        const TimeValuePoint &p2) const {
            if (p1.frame != p2.frame) return p1.frame < p2.frame;
            if (p1.value != p2.value) return p1.value < p2.value;
            return p1.label < p2.label;
        }
    };
    
    struct OrderComparator {
        bool operator()(const TimeValuePoint &p1,
                        const TimeValuePoint &p2) const {
            return p1.frame < p2.frame;
        }
    };
};


class SparseTimeValueModel : public SparseValueModel<TimeValuePoint>
{
    Q_OBJECT
    
public:
    SparseTimeValueModel(sv_samplerate_t sampleRate, int resolution, bool notifyOnAdd = true) :
        SparseValueModel<TimeValuePoint>(sampleRate, resolution, notifyOnAdd)
    {
        // Model is playable, but may not sound (if units not Hz or
        // range unsuitable)
        PlayParameterRepository::getInstance()->addPlayable(this);
    }

    SparseTimeValueModel(sv_samplerate_t sampleRate, int resolution, float valueMinimum, float valueMaximum,
                         bool notifyOnAdd = true) :
        SparseValueModel<TimeValuePoint>(sampleRate, resolution, valueMinimum, valueMaximum, notifyOnAdd)
    {
        // Model is playable, but may not sound (if units not Hz or
        // range unsuitable)
        PlayParameterRepository::getInstance()->addPlayable(this);
    }

    virtual ~SparseTimeValueModel()
    {
        PlayParameterRepository::getInstance()->removePlayable(this);
    }

    QString getTypeName() const { return tr("Sparse Time-Value"); }
    std::string getType() const { return "SparseTimeValueModel"; }

    virtual bool canPlay() const { return true; }
    virtual bool getDefaultPlayAudible() const { return false; } // user must unmute

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
        case 2: return tr("Value");
        case 3: return tr("Label");
        default: return tr("Unknown");
        }
    }

    virtual QVariant getData(int row, int column, int role) const
    {
        if (column < 2) {
            return SparseValueModel<TimeValuePoint>::getData
                    (row, column, role);
        }

        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return QVariant();

        switch (column) {
        case 2:
            if (role == Qt::EditRole || role == SortRole) return i->value;
            else return QString("%1 %2").arg(i->value).arg(getScaleUnits());
        case 3: return i->label;
        default: return QVariant();
        }
    }

    virtual UndoableCommand *getSetDataCommand(int row, int column, const QVariant &value, int role)
    {
        if (column < 2) {
            return SparseValueModel<TimeValuePoint>::getSetDataCommand
                    (row, column, value, role);
        }

        if (role != Qt::EditRole) return 0;
        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return 0;
        EditCommand *command = new EditCommand(this, tr("Edit Data"));

        Point point(*i);
        command->deletePoint(point);

        switch (column) {
        case 2: point.value = float(value.toDouble()); break;
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

