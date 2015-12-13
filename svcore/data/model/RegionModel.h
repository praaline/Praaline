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

#ifndef _REGION_MODEL_H_
#define _REGION_MODEL_H_

#include "IntervalModel.h"
#include "base/RealTime.h"

/**
 * RegionModel -- a concrete IntervalModel for intervals associated
 * with a value, which we call regions for no very compelling reason.
 */

/**
 * Region "point" type.  A region is something that has an onset time,
 * a single value, and a duration.  Like other points, it can also
 * have a label.
 *
 * This is called RegionRec instead of Region to avoid name collisions
 * with the X11 Region struct.  Bah.
 */

struct RegionRec
{
public:
    RegionRec() : frame(0), value(0.f), duration(0) { }
    RegionRec(sv_frame_t _frame) : frame(_frame), value(0.0f), duration(0) { }
    RegionRec(sv_frame_t _frame, float _value, sv_frame_t _duration, QString _label) :
	frame(_frame), value(_value), duration(_duration), label(_label) { }

    int getDimensions() const { return 3; }

    sv_frame_t frame;
    float value;
    sv_frame_t duration;
    QString label;

    QString getLabel() const { return label; }
    
    void toXml(QTextStream &stream,
               QString indent = "",
               QString extraAttributes = "") const
    {
	stream <<
            QString("%1<point frame=\"%2\" value=\"%3\" duration=\"%4\" label=\"%5\" %6/>\n")
	    .arg(indent).arg(frame).arg(value).arg(duration)
            .arg(XmlExportable::encodeEntities(label)).arg(extraAttributes);
    }

    QString toDelimitedDataString(QString delimiter, DataExportOptions, sv_samplerate_t sampleRate) const
    {
        QStringList list;
        list << RealTime::frame2RealTime(frame, sampleRate).toString().c_str();
        list << QString("%1").arg(value);
        list << RealTime::frame2RealTime(duration, sampleRate).toString().c_str();
        if (label != "") list << label;
        return list.join(delimiter);
    }

    struct Comparator {
	bool operator()(const RegionRec &p1,
			const RegionRec &p2) const {
	    if (p1.frame != p2.frame) return p1.frame < p2.frame;
	    if (p1.value != p2.value) return p1.value < p2.value;
	    if (p1.duration != p2.duration) return p1.duration < p2.duration;
	    return p1.label < p2.label;
	}
    };
    
    struct OrderComparator {
	bool operator()(const RegionRec &p1,
			const RegionRec &p2) const {
	    return p1.frame < p2.frame;
	}
    };
};


class RegionModel : public IntervalModel<RegionRec>
{
    Q_OBJECT
    
public:
    RegionModel(sv_samplerate_t sampleRate, int resolution,
                bool notifyOnAdd = true) :
	IntervalModel<RegionRec>(sampleRate, resolution, notifyOnAdd),
	m_valueQuantization(0),
        m_haveDistinctValues(false)
    {
    }

    RegionModel(sv_samplerate_t sampleRate, int resolution,
                float valueMinimum, float valueMaximum,
                bool notifyOnAdd = true) :
	IntervalModel<RegionRec>(sampleRate, resolution,
                            valueMinimum, valueMaximum,
                            notifyOnAdd),
	m_valueQuantization(0),
        m_haveDistinctValues(false)
    {
    }

    virtual ~RegionModel()
    {
    }

    float getValueQuantization() const { return m_valueQuantization; }
    void setValueQuantization(float q) { m_valueQuantization = q; }

    bool haveDistinctValues() const { return m_haveDistinctValues; }

    QString getTypeName() const { return tr("Region"); }
    std::string getType() const { return "RegionModel"; }

    virtual void toXml(QTextStream &out,
                       QString indent = "",
                       QString extraAttributes = "") const
    {
        std::cerr << "RegionModel::toXml: extraAttributes = \"" 
                  << extraAttributes.toStdString() << std::endl;

        IntervalModel<RegionRec>::toXml
	    (out,
             indent,
	     QString("%1 subtype=\"region\" valueQuantization=\"%2\"")
	     .arg(extraAttributes).arg(m_valueQuantization));
    }

    /**
     * TabularModel methods.  
     */
    
    virtual int getColumnCount() const
    {
        return 5;
    }

    virtual QString getHeading(int column) const
    {
        switch (column) {
        case 0: return tr("Time");
        case 1: return tr("Frame");
        case 2: return tr("Value");
        case 3: return tr("Duration");
        case 4: return tr("Label");
        default: return tr("Unknown");
        }
    }

    virtual QVariant getData(int row, int column, int role) const
    {
        if (column < 4) {
            return IntervalModel<RegionRec>::getData(row, column, role);
        }

        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return QVariant();

        switch (column) {
        case 4: return i->label;
        default: return QVariant();
        }
    }

    virtual SVCommand *getSetDataCommand(int row, int column, const QVariant &value, int role)
    {
        if (column < 4) {
            return IntervalModel<RegionRec>::getSetDataCommand
                (row, column, value, role);
        }

        if (role != Qt::EditRole) return 0;
        PointListIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return 0;
        EditCommand *command = new EditCommand(this, tr("Edit Data"));

        Point point(*i);
        command->deletePoint(point);

        switch (column) {
        case 4: point.label = value.toString(); break;
        }

        command->addPoint(point);
        return command->finish();
    }

    virtual SortType getSortType(int column) const
    {
        if (column == 4) return SortAlphabetical;
        return SortNumeric;
    }

    virtual void addPoint(const Point &point)
    {
        if (point.value != 0.f) m_haveDistinctValues = true;
        IntervalModel<RegionRec>::addPoint(point);
    }
    
protected:
    float m_valueQuantization;
    bool m_haveDistinctValues;
};

#endif
