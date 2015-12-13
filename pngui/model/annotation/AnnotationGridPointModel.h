#ifndef ANNOTATIONGRIDPOINTMODEL_H
#define ANNOTATIONGRIDPOINTMODEL_H

#include "pncore/base/RealTime.h"
#include "svcore/base/XmlExportable.h"
#include "svcore/data/model/SparseModel.h"

#include <QStringList>

/**
 * AnnotationGridPoint for use in a SparseModel. It represents a point in the timeline
 * where one or more annotation levels have a boundary (tier boundary).
 */

struct AnnotationGridPoint : public XmlExportable
{
public:
    AnnotationGridPoint(sv_frame_t _frame) :
        frame(_frame) { }
    AnnotationGridPoint(sv_frame_t _frame, sv_frame_t _duration, const QString &_speakerID, const QString &_levelID, int _itemNo,
                        const QString &_label = QString()) :
        frame(_frame), duration(_duration), speakerID(_speakerID), levelID(_levelID), itemNo(_itemNo), label(_label) { }

    int getDimensions() const { return 5; }

    sv_frame_t frame;
    sv_frame_t duration;
    QString speakerID;
    QString levelID;
    QString label;
    int itemNo;

    QString getLabel() const { return (label.isEmpty()) ? QString("%1 %2 %3").arg(speakerID).arg(levelID).arg(itemNo) : label; }

    void toXml(QTextStream &stream, QString indent = "",
               QString extraAttributes = "") const
    {
        stream << QString("%1<point frame=\"%2\" duration=\"%3\" speakerID=\"%4\" levelID=\"%5\" itemNo=\"%6\" %7 />\n")
                  .arg(indent).arg(frame).arg(duration).arg(speakerID).arg(levelID).arg(itemNo).arg(extraAttributes);
    }

    QString toDelimitedDataString(QString delimiter, DataExportOptions, sv_samplerate_t sampleRate) const
    {
        QStringList list;
        list << RealTime::frame2RealTime(frame, sampleRate).toString().c_str();
        list << RealTime::frame2RealTime(duration, sampleRate).toString().c_str();
        list << speakerID << levelID << QString("%1").arg(itemNo);
        return list.join(delimiter);
    }

    struct Comparator {
        bool operator()(const AnnotationGridPoint &p1, const AnnotationGridPoint &p2) const {
            if (p1.frame != p2.frame) return p1.frame < p2.frame;
            if (p1.speakerID != p2.speakerID) return p1.speakerID < p2.speakerID;
            if (p1.levelID != p2.levelID) return p1.levelID < p2.levelID;
            return p1.itemNo < p2.itemNo;
        }
    };

    struct OrderComparator {
        bool operator()(const AnnotationGridPoint &p1, const AnnotationGridPoint &p2) const {
            return p1.frame < p2.frame;
        }
    };
};


// Make this a class rather than a typedef so it can be predeclared.

class AnnotationGridPointModel : public SparseModel<AnnotationGridPoint>
{
    Q_OBJECT

public:
    AnnotationGridPointModel(sv_samplerate_t sampleRate, int resolution, bool notifyOnAdd = true) :
        SparseModel<AnnotationGridPoint>(sampleRate, resolution, notifyOnAdd)
    { }

    virtual void toXml(QTextStream &out,
                       QString indent = "",
                       QString extraAttributes = "") const
    {
        SparseModel<AnnotationGridPoint>::toXml(out, indent,
                                                QString("%1 subtype=\"annotationgridpoint\"").arg(extraAttributes));
    }

    QString getTypeName() const { return tr("Annotation Grid Point"); }

    /**
     * TabularModel methods.
     */

    virtual int getColumnCount() const
    {
        return 6;
    }

    virtual QString getHeading(int column) const
    {
        switch (column) {
        case 0: return tr("Time");
        case 1: return tr("Frame");
        case 2: return tr("Duration");
        case 3: return tr("Speaker ID");
        case 4: return tr("Level ID");
        case 5: return tr("Item No");
        default: return tr("Unknown");
        }
    }

    virtual QVariant getData(int row, int column, int role) const
    {
        if (column < 2) {
            return SparseModel<AnnotationGridPoint>::getData(row, column, role);
        }
        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return QVariant();

        if (column == 2) {
            return i->duration;
        } else if (column == 3) {
            return i->speakerID;
        } else if (column == 4) {
            return i->levelID;
        } else if (column == 5) {
            return i->itemNo;
        }
        return QVariant();
    }

    virtual SVCommand *getSetDataCommand(int row, int column, const QVariant &value, int role)
    {
        if (column < 2) {
            return SparseModel<AnnotationGridPoint>::getSetDataCommand(row, column, value, role);
        }
        // This model is for internal use and not supposed to be editable
        return 0;
    }

    virtual bool isColumnTimeValue(int column) const
    {
        return (column < 2);
    }

    virtual SortType getSortType(int column) const
    {
        if (column == 3 || column == 4) return SortAlphabetical;
        return SortNumeric;
    }

};

#endif // ANNOTATIONGRIDPOINTMODEL_H
