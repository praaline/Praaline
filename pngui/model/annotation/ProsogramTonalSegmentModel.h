#ifndef PROSOGRAMTONALSEGMENTMODEL_H
#define PROSOGRAMTONALSEGMENTMODEL_H

#include "pncore/base/RealTime.h"
#include "svcore/system/System.h"
#include "svcore/base/XmlExportable.h"
#include "svcore/data/model/SparseModel.h"

#include <QStringList>
#include <math.h>

/**
 * A tonal segment in Prosogram. Has a starting and ending frequency (in Hz) and a duration.
 * Handy conversion functions to ST (re 1 Hz) are provided.
 */

struct ProsogramTonalSegment : public XmlExportable
{
public:
    ProsogramTonalSegment(sv_frame_t _frame) :
        frame(_frame) { }
    ProsogramTonalSegment(sv_frame_t _frame, sv_frame_t _duration, const QString &_speakerID, double _f0StartHz, double _f0EndHz, int _index) :
        frame(_frame), duration(_duration), speakerID(_speakerID), f0StartHz(_f0StartHz), f0EndHz(_f0EndHz), index(_index) {}

    int getDimensions() const { return 6; }

    sv_frame_t frame;
    sv_frame_t duration;
    QString speakerID;
    double f0StartHz;
    double f0EndHz;
    int index;

    double f0StartST() const { return 12.0 * log2(f0StartHz); }
    double f0EndST() const   { return 12.0 * log2(f0EndHz);   }

    double movement() const  { return f0EndST() - f0StartST(); }
    bool rising(double threshold = 0.0) const { return (movement() > threshold); }
    bool falling(double threshold = 0.0) const { return (movement() < -threshold); }

    QString getLabel() const {
        return QString("Spk: %1 ToneSeg %2: from %3 Hz to %4 Hz Dur: %5 frames")
                .arg(speakerID).arg(index).arg(f0StartHz).arg(f0EndHz).arg(duration);
    }

    void toXml(QTextStream &stream, QString indent = "", QString extraAttributes = "") const
    {
        stream << QString("%1<point frame=\"%2\" duration=\"%3\" speakerID=\"%4\" f0StartHz=\"%5\" f0EndHz=\"%6\" index=\"%7\" %8 />\n")
                  .arg(indent).arg(frame).arg(duration).arg(speakerID).arg(f0StartHz).arg(f0EndHz).arg(index).arg(extraAttributes);
    }

    QString toDelimitedDataString(QString delimiter, DataExportOptions, sv_samplerate_t sampleRate) const
    {
        QStringList list;
        list << RealTime::frame2RealTime(frame, sampleRate).toString().c_str();
        list << RealTime::frame2RealTime(duration, sampleRate).toString().c_str();
        list << speakerID << QString("%1").arg(f0StartHz) << QString("%1").arg(f0EndHz) << QString("%1").arg(index);
        return list.join(delimiter);
    }

    struct Comparator {
        bool operator()(const ProsogramTonalSegment &p1, const ProsogramTonalSegment &p2) const {
            if (p1.frame != p2.frame) return p1.frame < p2.frame;
            if (p1.speakerID != p2.speakerID) return p1.speakerID < p2.speakerID;
            if (p1.duration != p2.duration) return p1.duration < p2.duration;
            return p1.index < p2.index;
        }
    };

    struct OrderComparator {
        bool operator()(const ProsogramTonalSegment &p1, const ProsogramTonalSegment &p2) const {
            return p1.frame < p2.frame;
        }
    };
};


// Make this a class rather than a typedef so it can be predeclared.

class ProsogramTonalSegmentModel : public SparseModel<ProsogramTonalSegment>
{
    Q_OBJECT

public:
    ProsogramTonalSegmentModel(sv_samplerate_t sampleRate, int resolution, bool notifyOnAdd = true) :
        SparseModel<ProsogramTonalSegment>(sampleRate, resolution, notifyOnAdd),
        m_f0MinimumHz(0.0), m_f0MaximumHz(0.0), m_haveExtents(false)
    { }
    ProsogramTonalSegmentModel(sv_samplerate_t sampleRate, int resolution,
                               double f0MinimumHz, double f0MaximumHz, bool notifyOnAdd = true) :
        SparseModel<ProsogramTonalSegment>(sampleRate, resolution, notifyOnAdd),
        m_f0MinimumHz(f0MinimumHz), m_f0MaximumHz(f0MaximumHz), m_haveExtents(true)
    { }

    virtual void toXml(QTextStream &out,
                       QString indent = "",
                       QString extraAttributes = "") const
    {
        SparseModel<ProsogramTonalSegment>::toXml(out, indent,
                                                QString("%1 subtype=\"prosogram_tonal_segment\"").arg(extraAttributes));
    }

    QString getTypeName() const { return tr("Prosogram Tonal Segment"); }
    std::string getType() const { return "ProsogramTonalSegmentModel"; }

    virtual float getf0MinimumHz() const { return m_f0MinimumHz; }
    virtual float getf0MinimumST() const { return 12.0 * log2(m_f0MinimumHz); }
    virtual float getf0MaximumHz() const { return m_f0MaximumHz; }
    virtual float getf0MaximumST() const { return 12.0 * log2(m_f0MaximumHz); }

    virtual void addPoint(const ProsogramTonalSegment &point)
    {
        bool allChange = false;
        if (!ISNAN(point.f0StartHz) && !ISINF(point.f0StartHz)) {
            if (!m_haveExtents || point.f0StartHz < m_f0MinimumHz) {
                m_f0MinimumHz = point.f0StartHz; allChange = true;
            }
            if (!m_haveExtents || point.f0EndHz < m_f0MinimumHz) {
                m_f0MinimumHz = point.f0EndHz; allChange = true;
            }
            if (!m_haveExtents || point.f0StartHz > m_f0MaximumHz) {
                m_f0MaximumHz = point.f0StartHz; allChange = true;
            }
            if (!m_haveExtents || point.f0EndHz > m_f0MaximumHz) {
                m_f0MaximumHz = point.f0EndHz; allChange = true;
            }
            m_haveExtents = true;
        }
        SparseModel<ProsogramTonalSegment>::addPoint(point);
        if (allChange) emit modelChanged();
    }

    virtual void deletePoint(const ProsogramTonalSegment &point)
    {
        SparseModel<ProsogramTonalSegment>::deletePoint(point);

        if (point.f0StartHz == m_f0MinimumHz || point.f0EndHz == m_f0MinimumHz ||
            point.f0StartHz == m_f0MaximumHz || point.f0EndHz == m_f0MaximumHz) {

            double formerMin = m_f0MinimumHz, formerMax = m_f0MaximumHz;

            for (typename SparseModel<ProsogramTonalSegment>::PointList::const_iterator i = m_points.begin();
                 i != m_points.end(); ++i) {

                if (i == m_points.begin() || i->f0StartHz < m_f0MinimumHz) {
                    m_f0MinimumHz = i->f0StartHz;
                }
                if (i->f0EndHz < m_f0MinimumHz) {
                    m_f0MinimumHz = i->f0EndHz;
                }
                if (i == m_points.begin() || i->f0StartHz > m_f0MaximumHz) {
                    m_f0MaximumHz = i->f0StartHz;
                }
                if (i->f0EndHz > m_f0MaximumHz) {
                    m_f0MaximumHz = i->f0EndHz;
                }
            }

            if (formerMin != m_f0MinimumHz || formerMax != m_f0MaximumHz) {
                emit modelChanged();
            }
        }
    }

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
        case 4: return tr("f0 start (Hz)");
        case 5: return tr("f0 end (Hz)");
        default: return tr("Unknown");
        }
    }

    virtual QVariant getData(int row, int column, int role) const
    {
        if (column < 2) {
            return SparseModel<ProsogramTonalSegment>::getData(row, column, role);
        }
        PointListConstIterator i = getPointListIteratorForRow(row);
        if (i == m_points.end()) return QVariant();

        if (column == 2) {
            return int(i->duration);
        } else if (column == 3) {
            return i->speakerID;
        } else if (column == 4) {
            return i->f0StartHz;
        } else if (column == 5) {
            return i->f0EndHz;
        }
        return QVariant();
    }

    virtual UndoableCommand *getSetDataCommand(int row, int column, const QVariant &value, int role)
    {
        if (column < 2) {
            return SparseModel<ProsogramTonalSegment>::getSetDataCommand(row, column, value, role);
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
        if (column == 3) return SortAlphabetical;
        return SortNumeric;
    }

protected:
    double m_f0MinimumHz;
    double m_f0MaximumHz;
    bool m_haveExtents;
};

#endif // PROSOGRAMTONALSEGMENTMODEL_H
