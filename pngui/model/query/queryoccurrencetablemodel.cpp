#include <QDebug>
#include <QString>
#include <QColor>
#include "pncore/query/querydefinition.h"
#include "pncore/query/queryoccurrence.h"
#include "pncore/corpus/corpus.h"
#include "queryoccurrencetablemodel.h"

QueryOccurrenceTableModel::QueryOccurrenceTableModel(QPointer<Corpus> corpus, QueryDefinition *queryDefinition,
                                                     QList<QueryOccurrencePointer *> &pointers,
                                                     bool multiline, QObject *parent) :
    QAbstractTableModel(parent), m_corpus(corpus), m_queryDefinition(queryDefinition), m_pointers(pointers), m_occurrences(QHash<int, QueryOccurrence *>()),
    m_multiLine(multiline)
{
}

QueryOccurrenceTableModel::~QueryOccurrenceTableModel()
{
    qDeleteAll(m_occurrences);
}

QModelIndex QueryOccurrenceTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int QueryOccurrenceTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!m_queryDefinition) return 0;
    int levattr = m_queryDefinition->resultLevelsAttributes.count();
    if (m_multiLine)
        return m_pointers.count() * levattr;
    else
        return m_pointers.count();
}

int QueryOccurrenceTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!m_queryDefinition) return 0;
    // corpusID, communicationID, annotationID, speakerID, tMin, tMax, Left Context + {sequences} + Right Context
    if (m_multiLine)
        return m_queryDefinition->longestSequenceLength() + 8;
    else
        return m_queryDefinition->longestSequenceLength() * m_queryDefinition->resultLevelsAttributes.count() + 8;
}

QVariant QueryOccurrenceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal && m_multiLine) {
        if      (section == 0)  return tr("Corpus ID");
        else if (section == 1)  return tr("Communication ID");
        else if (section == 2)  return tr("Annotation ID");
        else if (section == 3)  return tr("Speaker ID");
        else if (section == 4)  return tr("tMin");
        else if (section == 5)  return tr("tMax");
        else if (section == 6)  return tr("Left Context");
        else if (section >= 7 && section < columnCount() - 1)
            return QString("T%1").arg(section - 6);
        else if (section == columnCount() - 1)
            return tr("Right Context");
    }
    else if (orientation == Qt::Horizontal && !m_multiLine) {
        if      (section == 0)  return tr("Corpus ID");
        else if (section == 1)  return tr("Communication ID");
        else if (section == 2)  return tr("Annotation ID");
        else if (section == 3)  return tr("Speaker ID");
        else if (section == 4)  return tr("tMin");
        else if (section == 5)  return tr("tMax");
        else if (section == 6)  return tr("Left Context");
        else if (section >= 7 && section < columnCount() - 1) {
            if (m_queryDefinition->resultLevelsAttributes.count() == 0)
                return QVariant();
            int indexLevelAttribute = (section - 7) % m_queryDefinition->resultLevelsAttributes.count();
            int indexSeq = (section - 7) / m_queryDefinition->resultLevelsAttributes.count();
            if ((indexLevelAttribute < 0) || (indexLevelAttribute >= m_queryDefinition->resultLevelsAttributes.count()))
                return QVariant();
            QString attributeID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;
            if (!attributeID.isEmpty()) return QString("%1 %2").arg(attributeID).arg(indexSeq + 1);
            // else, it's a level
            QString levelID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
            return QString("%1 %2").arg(levelID).arg(indexSeq + 1);
        }
        else if (section == columnCount() - 1)
            return tr("Right Context");
    }
    else if (orientation == Qt::Vertical && m_multiLine) {
        if (m_queryDefinition->resultLevelsAttributes.count() == 0)
            return QVariant();
        int indexLevelAttribute = section % m_queryDefinition->resultLevelsAttributes.count();
        if ((indexLevelAttribute < 0) || (indexLevelAttribute >= m_queryDefinition->resultLevelsAttributes.count()))
            return QVariant();

        QString attributeID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;
        if (!attributeID.isEmpty()) return QString(" %1").arg(attributeID);
        // else, it's a level
        return m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
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
    if (m_queryDefinition->resultLevelsAttributes.count() == 0) return QVariant();

    int indexOccurrence = 0, indexLevelAttribute = 0, indexSeq = 0;
    if (m_multiLine) {
        indexOccurrence = index.row() / m_queryDefinition->resultLevelsAttributes.count();
        indexLevelAttribute = index.row() % m_queryDefinition->resultLevelsAttributes.count();
        indexSeq = index.column() - 7;
        // qDebug() << "Multiline format: occurrence " << indexOccurrence << " lev-attr " << indexLevelAttribute << " seq " << indexSeq;
    } else {
        indexOccurrence = index.row();
        indexLevelAttribute = (index.column() - 7) % m_queryDefinition->resultLevelsAttributes.count();
        indexSeq = (index.column() - 7) / m_queryDefinition->resultLevelsAttributes.count();
        // qDebug() << "Exploded format: occurrence " << indexOccurrence << " lev-attr " << indexLevelAttribute << " seq " << indexSeq;
    }

    if ((indexOccurrence < 0) || (indexOccurrence >= m_pointers.count())) return QVariant();
    QueryOccurrence *occurrence = m_occurrences.value(indexOccurrence, 0);
    if (!occurrence) {
        // lazy loading
        occurrence = m_corpus->datastoreAnnotations()->getOccurrence(m_pointers.at(indexOccurrence), m_queryDefinition);
        m_occurrences.insert(indexOccurrence, occurrence);
        // qDebug() << "loaded " << indexOccurrence;
    }
    if (!occurrence) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if      (index.column() == 0)   return occurrence->corpusID();
        else if (index.column() == 1)   return occurrence->communicationID();
        else if (index.column() == 2)   return occurrence->annotationID();
        else if (index.column() == 3)   return occurrence->speakerIDs();
        else if (index.column() == 4)   return occurrence->tMin().toDouble();
        else if (index.column() == 5)   return occurrence->tMax().toDouble();
        else if (index.column() == 6) {
            // Left context
            if (m_leftContext.contains(indexOccurrence)) {
                return m_leftContext.value(indexOccurrence);
            } else {
                QString leftContext;
                if (m_multiLine) {
                    if ((indexLevelAttribute < 0) || (indexLevelAttribute >= m_queryDefinition->resultLevelsAttributes.count()))
                        return QVariant();
                    QString levelID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
                    QString attributeID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;
                    leftContext = concatenate(occurrence->leftContext(levelID), attributeID);
                }
                else
                    leftContext = concatenate(occurrence->leftContext(m_queryDefinition->resultLevelsAttributes.first().first), "");
                m_leftContext.insert(indexOccurrence, leftContext);
                return leftContext;
            }
        }
        else if (index.column() >= 7 && index.column() < columnCount() - 1) {
            // Target
            if ((indexLevelAttribute < 0) || (indexLevelAttribute >= m_queryDefinition->resultLevelsAttributes.count()))
                return QVariant();
            if ((indexSeq < 0) || (indexSeq >= m_queryDefinition->longestSequenceLength()))
                return QVariant();
            QString levelID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
            QString attributeID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;

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
            if (m_rightContext.contains(indexOccurrence)) {
                return m_rightContext.value(indexOccurrence);
            }
            else {
                QString rightContext;
                if (m_multiLine) {
                    if ((indexLevelAttribute < 0) || (indexLevelAttribute >= m_queryDefinition->resultLevelsAttributes.count()))
                        return QVariant();
                    QString levelID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
                    QString attributeID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;
                    rightContext = concatenate(occurrence->rightContext(levelID), attributeID);
                }
                else
                    rightContext = concatenate(occurrence->rightContext(m_queryDefinition->resultLevelsAttributes.first().first), "");
                m_rightContext.insert(indexOccurrence, rightContext);
                return rightContext;
            }
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        if (index.column() >= 4 && index.column() <= 6) // tMin, tMax, left context
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        else if (index.column() >= 7 && index.column() < columnCount() - 1)
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
    if (index.column() >= 7 && index.column() < columnCount() - 1)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable;
}

bool QueryOccurrenceTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    if (index.column() < 7 || index.column() == columnCount() - 1) return false;

    int indexOccurrence = 0, indexLevelAttribute = 0, indexSeq = 0;
    if (m_multiLine) {
        indexOccurrence = index.row() / m_queryDefinition->resultLevelsAttributes.count();
        indexLevelAttribute = index.row() % m_queryDefinition->resultLevelsAttributes.count();
        indexSeq = index.column() - 7;
        // qDebug() << "Write Multiline format: occurrence " << indexOccurrence << " lev-attr " << indexLevelAttribute << " seq " << indexSeq;
    } else {
        indexOccurrence = index.row();
        indexLevelAttribute = (index.column() - 7) % m_queryDefinition->resultLevelsAttributes.count();
        indexSeq = (index.column() - 7) / m_queryDefinition->resultLevelsAttributes.count();
        // qDebug() << "Write Exploded format: occurrence " << indexOccurrence << " lev-attr " << indexLevelAttribute << " seq " << indexSeq;
    }

    if ((indexOccurrence < 0) || (indexOccurrence >= m_pointers.count())) return false;
    if (indexLevelAttribute >= m_queryDefinition->resultLevelsAttributes.count()) return false;
    if (indexSeq >= m_queryDefinition->longestSequenceLength()) return false;

    QueryOccurrence *occurrence = m_occurrences.value(indexOccurrence, 0);
    if (!occurrence) {
        // lazy loading
        occurrence = m_corpus->datastoreAnnotations()->getOccurrence(m_pointers.at(indexOccurrence), m_queryDefinition);
        m_occurrences.insert(indexOccurrence, occurrence);
        qDebug() << "loaded " << indexOccurrence;
    }
    if (!occurrence) return false;

    QString levelID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).first;
    QString attributeID = m_queryDefinition->resultLevelsAttributes.at(indexLevelAttribute).second;

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

