#include <QDebug>
#include <QString>
#include <QColor>

#include "pncore/query/QueryDefinition.h"
#include "pncore/query/QueryOccurrence.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "QueryOccurrenceTableModel.h"

struct QueryOccurrenceTableModelData {
    QueryOccurrenceTableModelData() : repository(0), multiLine(false) {}

    QPointer<CorpusRepository> repository;
    QueryDefinition *queryDefinition;
    QList<QueryOccurrencePointer *> pointers;
    mutable QHash<int, QueryOccurrence *> occurrences;
    mutable QHash<int, QString> leftContext;
    mutable QHash<int, QString> rightContext;
    bool multiLine;
};

QueryOccurrenceTableModel::QueryOccurrenceTableModel(CorpusRepository *repository, QueryDefinition *queryDefinition,
                                                     QList<QueryOccurrencePointer *> &pointers,
                                                     bool multiline, QObject *parent) :
    QAbstractTableModel(parent), d(new QueryOccurrenceTableModelData)
{
    d->repository = repository;
    d->queryDefinition = queryDefinition;
    d->pointers = pointers;
    d->occurrences = QHash<int, QueryOccurrence *>();
    d->multiLine = multiline;
}

QueryOccurrenceTableModel::~QueryOccurrenceTableModel()
{
    qDeleteAll(d->occurrences);
    delete d;
}

QModelIndex QueryOccurrenceTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int QueryOccurrenceTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!d->queryDefinition) return 0;
    int levattr = d->queryDefinition->resultLevelsAttributes.count();
    if (d->multiLine)
        return d->pointers.count() * levattr;
    else
        return d->pointers.count();
}

int QueryOccurrenceTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!d->queryDefinition) return 0;
    // corpusID, communicationID, annotationID, speakerID, interval No, tMin, tMax, Left Context + {sequences} + Right Context
    if (d->multiLine)
        return d->queryDefinition->longestSequenceLength() + 9;
    else
        return d->queryDefinition->longestSequenceLength() * d->queryDefinition->resultLevelsAttributes.count() + 9;
}

QVariant QueryOccurrenceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal && d->multiLine) {
        if      (section == 0)  return tr("Corpus ID");
        else if (section == 1)  return tr("Communication ID");
        else if (section == 2)  return tr("Annotation ID");
        else if (section == 3)  return tr("Speaker ID");
        else if (section == 4)  return tr("Interval No");
        else if (section == 5)  return tr("tMin");
        else if (section == 6)  return tr("tMax");
        else if (section == 7)  return tr("Left Context");
        else if (section >= 8 && section < columnCount() - 1)
            return QString("T%1").arg(section - 6);
        else if (section == columnCount() - 1)
            return tr("Right Context");
    }
    else if (orientation == Qt::Horizontal && !d->multiLine) {
        if      (section == 0)  return tr("Corpus ID");
        else if (section == 1)  return tr("Communication ID");
        else if (section == 2)  return tr("Annotation ID");
        else if (section == 3)  return tr("Speaker ID");
        else if (section == 4)  return tr("Interval No");
        else if (section == 5)  return tr("tMin");
        else if (section == 6)  return tr("tMax");
        else if (section == 7)  return tr("Left Context");
        else if (section >= 8 && section < columnCount() - 1) {
            if (d->queryDefinition->resultLevelsAttributes.count() == 0)
                return QVariant();
            int indexLevelAttribute = (section - 8) % d->queryDefinition->resultLevelsAttributes.count();
            int indexSeq = (section - 8) / d->queryDefinition->resultLevelsAttributes.count();
            if ((indexLevelAttribute < 0) || (indexLevelAttribute >= d->queryDefinition->resultLevelsAttributes.count()))
                return QVariant();
            QString attributeID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;
            if (!attributeID.isEmpty()) return QString("%1 %2").arg(attributeID).arg(indexSeq + 1);
            // else, it's a level
            QString levelID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
            return QString("%1 %2").arg(levelID).arg(indexSeq + 1);
        }
        else if (section == columnCount() - 1)
            return tr("Right Context");
    }
    else if (orientation == Qt::Vertical && d->multiLine) {
        if (d->queryDefinition->resultLevelsAttributes.count() == 0)
            return QVariant();
        int indexLevelAttribute = section % d->queryDefinition->resultLevelsAttributes.count();
        if ((indexLevelAttribute < 0) || (indexLevelAttribute >= d->queryDefinition->resultLevelsAttributes.count()))
            return QVariant();

        QString attributeID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;
        if (!attributeID.isEmpty()) return QString(" %1").arg(attributeID);
        // else, it's a level
        return d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
    }
    return QVariant();
}

QString concatenate(const QList<QueryOccurrence::ResultInterval> &intervals, const QString &attributeID)
{
    QString ret;
    foreach (QueryOccurrence::ResultInterval interval, intervals) {
        if (!interval.interval) continue;
        if (attributeID.isEmpty())
            ret.append(interval.interval->text()).append(" ");
        else
            ret.append(interval.interval->attribute(attributeID).toString()).append(" ");
    }
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

QVariant QueryOccurrenceTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= rowCount()) return QVariant();
    if (d->queryDefinition->resultLevelsAttributes.count() == 0) return QVariant();

    int indexOccurrence = 0, indexLevelAttribute = 0, indexSeq = 0;
    if (d->multiLine) {
        indexOccurrence = index.row() / d->queryDefinition->resultLevelsAttributes.count();
        indexLevelAttribute = index.row() % d->queryDefinition->resultLevelsAttributes.count();
        indexSeq = index.column() - 8;
        // qDebug() << "Multiline format: occurrence " << indexOccurrence << " lev-attr " << indexLevelAttribute << " seq " << indexSeq;
    } else {
        indexOccurrence = index.row();
        indexLevelAttribute = (index.column() - 8) % d->queryDefinition->resultLevelsAttributes.count();
        indexSeq = (index.column() - 8) / d->queryDefinition->resultLevelsAttributes.count();
        // qDebug() << "Exploded format: occurrence " << indexOccurrence << " lev-attr " << indexLevelAttribute << " seq " << indexSeq;
    }

    if ((indexOccurrence < 0) || (indexOccurrence >= d->pointers.count())) return QVariant();
    QueryOccurrence *occurrence = d->occurrences.value(indexOccurrence, 0);
    if (!occurrence) {
        // lazy loading
        occurrence = d->repository->annotations()->getOccurrence(d->pointers.at(indexOccurrence), d->queryDefinition,
                                                                 d->queryDefinition->lengthContextLeft, d->queryDefinition->lengthContextRight);
        d->occurrences.insert(indexOccurrence, occurrence);
        // qDebug() << "loaded " << indexOccurrence;
    }
    if (!occurrence) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if      (index.column() == 0)   return occurrence->corpusID();
        else if (index.column() == 1)   return occurrence->communicationID();
        else if (index.column() == 2)   return occurrence->annotationID();
        else if (index.column() == 3)   return occurrence->speakerIDs();
        else if (index.column() == 4)   return occurrence->intervalNoTarget();
        else if (index.column() == 5)   return occurrence->tMin().toDouble();
        else if (index.column() == 6)   return occurrence->tMax().toDouble();
        else if (index.column() == 7) {
            // Left context
            if (d->leftContext.contains(index.row())) {
                return d->leftContext.value(index.row());
            } else {
                QString leftContext;
                if (d->multiLine) {
                    if ((indexLevelAttribute < 0) || (indexLevelAttribute >= d->queryDefinition->resultLevelsAttributes.count()))
                        return QVariant();
                    QString levelID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
                    QString attributeID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;
                    leftContext = concatenate(occurrence->leftContext(levelID), attributeID);
                }
                else
                    leftContext = concatenate(occurrence->leftContext(d->queryDefinition->resultLevelsAttributes.first().first), "");
                d->leftContext.insert(index.row(), leftContext);
                return leftContext;
            }
        }
        else if (index.column() >= 8 && index.column() < columnCount() - 1) {
            // Target
            if ((indexLevelAttribute < 0) || (indexLevelAttribute >= d->queryDefinition->resultLevelsAttributes.count()))
                return QVariant();
            if ((indexSeq < 0) || (indexSeq >= d->queryDefinition->longestSequenceLength()))
                return QVariant();
            QString levelID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
            QString attributeID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;

            QList<QueryOccurrence::ResultInterval> intervals = occurrence->target(levelID);
            if ((indexSeq < 0) || (indexSeq >= intervals.count())) return QVariant();
            Interval *intv = intervals.at(indexSeq).interval;
            if (!intv) return QVariant();
            if (attributeID.isEmpty())
                return intv->text();
            else
                return intv->attribute(attributeID);
        }
        else if (index.column() == columnCount() - 1) {
            // Right context
            if (d->rightContext.contains(index.row())) {
                return d->rightContext.value(index.row());
            }
            else {
                QString rightContext;
                if (d->multiLine) {
                    if ((indexLevelAttribute < 0) || (indexLevelAttribute >= d->queryDefinition->resultLevelsAttributes.count()))
                        return QVariant();
                    QString levelID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
                    QString attributeID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;
                    rightContext = concatenate(occurrence->rightContext(levelID), attributeID);
                }
                else
                    rightContext = concatenate(occurrence->rightContext(d->queryDefinition->resultLevelsAttributes.first().first), "");
                d->rightContext.insert(index.row(), rightContext);
                return rightContext;
            }
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        if (index.column() >= 4 && index.column() <= 7) // interval No, tMin, tMax, left context
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        else if (index.column() >= 8 && index.column() < columnCount() - 1)
            return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    else if (role == Qt::BackgroundColorRole) {
        if (indexOccurrence % 2) return QColor(QColor::colorNames().at(1));
    }
    return QVariant();
}

Qt::ItemFlags QueryOccurrenceTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    if (index.column() >= 8 && index.column() < columnCount() - 1)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable;
}

bool QueryOccurrenceTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    if (index.column() < 8 || index.column() == columnCount() - 1) return false;

    int indexOccurrence = 0, indexLevelAttribute = 0, indexSeq = 0;
    if (d->multiLine) {
        indexOccurrence = index.row() / d->queryDefinition->resultLevelsAttributes.count();
        indexLevelAttribute = index.row() % d->queryDefinition->resultLevelsAttributes.count();
        indexSeq = index.column() - 8;
        // qDebug() << "Write Multiline format: occurrence " << indexOccurrence << " lev-attr " << indexLevelAttribute << " seq " << indexSeq;
    } else {
        indexOccurrence = index.row();
        indexLevelAttribute = (index.column() - 8) % d->queryDefinition->resultLevelsAttributes.count();
        indexSeq = (index.column() - 8) / d->queryDefinition->resultLevelsAttributes.count();
        // qDebug() << "Write Exploded format: occurrence " << indexOccurrence << " lev-attr " << indexLevelAttribute << " seq " << indexSeq;
    }

    if ((indexOccurrence < 0) || (indexOccurrence >= d->pointers.count())) return false;
    if (indexLevelAttribute >= d->queryDefinition->resultLevelsAttributes.count()) return false;
    if (indexSeq >= d->queryDefinition->longestSequenceLength()) return false;

    QueryOccurrence *occurrence = d->occurrences.value(indexOccurrence, 0);
    if (!occurrence) {
        // lazy loading
        occurrence = d->repository->annotations()->getOccurrence(d->pointers.at(indexOccurrence), d->queryDefinition,
                                                                 d->queryDefinition->lengthContextLeft, d->queryDefinition->lengthContextRight);
        d->occurrences.insert(indexOccurrence, occurrence);
        qDebug() << "loaded " << indexOccurrence;
    }
    if (!occurrence) return false;

    QString levelID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
    QString attributeID = d->queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;

    QList<QueryOccurrence::ResultInterval> intervals = occurrence->target(levelID);
    if ((indexSeq < 0) || (indexSeq >= intervals.count())) return false;
    Interval *intv = intervals.at(indexSeq).interval;
    if (!intv) return false;
    if (attributeID.isEmpty())
        intv->setText(value.toString());
    else
        intv->setAttribute(attributeID, value);
    return true;
}

