#ifndef DIFFSESFORINTERVALSTABLEMODEL_H
#define DIFFSESFORINTERVALSTABLEMODEL_H

#include <QObject>
#include <QList>
#include <QColor>
#include <QAbstractTableModel>
#include "pncore/annotation/Interval.h"
using namespace std;
#include "pnlib/diff/dtl.h"
using namespace dtl;
#include "pnlib/diff/diffintervals.h"

class DiffSESforIntervalsTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DiffSESforIntervalsTableModel(dtl::Ses<Praaline::Core::Interval *>::sesElemVec &sesElemVec,
                                           const QList<Praaline::Core::Interval *> &intervals_A,
                                           const QList<Praaline::Core::Interval *> &intervals_B,
                                           const QString &attributeID_A = QString(), const QString &attributeID_B = QString(),
                                           const QStringList &extraAttributeIDs_A = QStringList(),
                                           const QStringList &extraAttributeIDs_B = QStringList(),
                                           QObject *parent = nullptr);
    ~DiffSESforIntervalsTableModel();

    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setColors(const QColor &colorSame, const QColor &colorAddition, const QColor &colorDeletion);

signals:

public slots:

private:
    dtl::Ses<Praaline::Core::Interval *>::sesElemVec &m_sesElemVec;
    QList<Praaline::Core::Interval *> m_intervals_A;
    QList<Praaline::Core::Interval *> m_intervals_B;
    QString m_attributeID_A;
    QString m_attributeID_B;
    QStringList m_extraAttributeIDs_A;
    QStringList m_extraAttributeIDs_B;
    QColor m_colorAddition;
    QColor m_colorDeletion;
    QColor m_colorSame;
};

#endif // DIFFSESFORINTERVALSTABLEMODEL_H
