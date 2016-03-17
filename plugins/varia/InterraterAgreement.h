#ifndef INTERRATERAGREEMENT_H
#define INTERRATERAGREEMENT_H

#include <QString>
#include <QList>
#include <QPair>
#include <QHash>
#include <QMap>
#include <QStandardItemModel>
#include "pncore/annotation/interval.h"

class InterraterAgreement
{
public:
    InterraterAgreement();
    ~InterraterAgreement();

    void addGroup(QString groupName, QStringList labels, bool includeEmpty = false);
    void removeGroup(QString groupName);

    void reset();
    void resetCounts();
    void categorise(QString annotatorID, const QList<Interval *> &intervals, QString attributeID = QString());
    double getFleissKappa() const;
    QStandardItemModel *createTableModel();

    double getCohenKappa(const QString &groupName_1, const QString &groupName_2,
                         const QList<Interval *> &intervals,
                         QString attributeID_1 = QString(), QString attributeID_2 = QString()) const;

private:
    QHash<QString, QStringList> m_groups;
    QString m_groupForEmpty;
    QHash<QString, QString> m_classification;
    // A list of annotators (columns): each containing the count for a given category
    QMap<QString, QHash<QString, int> > m_counts;

    QString groupNameForLabel(const QString &label) const;
};

#endif // INTERRATERAGREEMENT_H
