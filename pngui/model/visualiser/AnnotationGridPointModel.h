#ifndef ANNOTATIONGRIDPOINTMODEL_H
#define ANNOTATIONGRIDPOINTMODEL_H

#include "PraalineCore/Base/RealTime.h"
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
    AnnotationGridPoint(sv_frame_t _frame, const QString &_speakerID, const QString &_levelID, const QString &_label = QString()) :
        frame(_frame), speakerID(_speakerID), levelID(_levelID), label(_label) { }

    int getDimensions() const { return 2; }

    sv_frame_t frame;
    QString speakerID;
    QString levelID;
    QString label;

    QString getLabel() const { return (label.isEmpty()) ? QString("%1 %2 %3").arg(speakerID, levelID, QString::number(frame)) : label; }

    void toXml(QTextStream &stream, QString indent = "", QString extraAttributes = "") const
    {
        stream << QString("%1<point frame=\"%2\" speakerID=\"%3\" levelID=\"%4\" label=\"%5\" %6 />\n")
                  .arg(indent, QString::number(frame), speakerID, levelID, label, extraAttributes);
    }

    QString toDelimitedDataString(QString delimiter, DataExportOptions, sv_samplerate_t sampleRate) const
    {
        QStringList list;
        list << RealTime::frame2RealTime(frame, sampleRate).toString().c_str();
        list << speakerID << levelID << label;
        return list.join(delimiter);
    }

    struct Comparator {
        bool operator()(const AnnotationGridPoint &p1, const AnnotationGridPoint &p2) const {
            if (p1.frame != p2.frame) return p1.frame < p2.frame;
            if (p1.speakerID != p2.speakerID) return p1.speakerID < p2.speakerID;
            return p1.levelID < p2.levelID;
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
        return 4;
    }

    virtual QString getHeading(int column) const
    {
        switch (column) {
        case 0: return tr("Time");
        case 1: return tr("Frame");
        case 2: return tr("Speaker ID");
        case 3: return tr("Level ID");
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
            return i->speakerID;
        } else if (column == 3) {
            return i->levelID;
        }
        return QVariant();
    }

    virtual UndoableCommand *getSetDataCommand(int row, int column, const QVariant &value, int role)
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
        if (column == 2 || column == 3) return SortAlphabetical;
        return SortNumeric;
    }

};

#endif // ANNOTATIONGRIDPOINTMODEL_H
