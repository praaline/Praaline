#include "InterraterAgreement.h"

InterraterAgreement::InterraterAgreement()
{
}

InterraterAgreement::~InterraterAgreement()
{
}

void InterraterAgreement::addGroup(QString groupName, QStringList labels, bool includeEmpty)
{
    if (m_groups.contains(groupName)) removeGroup(groupName);
    m_groups.insert(groupName, labels);
    foreach (QString label, labels) {
        m_classification.insert(label, groupName);
    }
    if (includeEmpty) m_groupForEmpty = groupName;
    m_counts.clear();
}

void InterraterAgreement::removeGroup(QString groupName)
{
    m_groups.remove(groupName);
    foreach (QString label, m_classification.keys()) {
        if (m_classification.value(label) == groupName) {
            m_classification.remove(label);
        }
    }
    if (m_groupForEmpty == groupName) m_groupForEmpty = "";
    m_counts.clear();
}

void InterraterAgreement::reset()
{
    m_groups.clear();
    m_classification.clear();
    m_groupForEmpty = "";
    m_counts.clear();
}

void InterraterAgreement::resetCounts()
{
    m_counts.clear();
}

QString InterraterAgreement::groupNameForLabel(const QString &label) const
{
    QString groupName;
    if ((label.isEmpty()) && (!m_groupForEmpty.isEmpty())) {
        groupName = m_groupForEmpty;
    }
    else if (m_classification.contains(label)) {
        groupName = m_classification.value(label);
    }
    return groupName;
}

void InterraterAgreement::categorise(QString annotatorID, const QList<Interval *> &intervals, QString attributeID)
{
    QHash<QString, int> count;
    foreach (Interval *intv, intervals) {
        QString label = (attributeID.isEmpty()) ? intv->text() : intv->attribute(attributeID).toString();
        QString groupName = groupNameForLabel(label);
        if (groupName.isEmpty())
            continue; // ignore
        else {
            if (count.contains(groupName))
                count[groupName] = count.value(groupName) + 1;
            else
                count.insert(groupName, 1);
        }
    }
    m_counts.insert(annotatorID, count);
}


double InterraterAgreement::getFleissKappa() const
{
    return -1.0;
}

QStandardItemModel *InterraterAgreement::createTableModel()
{
    QStandardItemModel *model = new QStandardItemModel();
    model->setRowCount(m_groups.count());
    model->setColumnCount(m_counts.count()); // number of annotators
    model->setHorizontalHeaderLabels(m_counts.keys());
    model->setVerticalHeaderLabels(m_groups.keys());
    int col = 0;
    foreach (QString annotatorID, m_counts.keys()) {
        QHash<QString, int> count = m_counts.value(annotatorID);
        int row = 0;
        foreach (QString groupName, m_groups.keys()) {
            QStandardItem *item;
            if (count.contains(groupName)) {
                item = new QStandardItem(QString::number(count.value(groupName)));
            }
            else {
                item = new QStandardItem("0");
            }
            item->setTextAlignment(Qt::AlignRight);
            model->setItem(row, col, item);
            row++;
        }
        col++;
    }
    return model;
}

double InterraterAgreement::getCohenKappa(const QString &groupName_1, const QString &groupName_2,
                                          const QList<Interval *> &intervals, QString attributeID_1, QString attributeID_2) const
{
    int n00 = 0, n01 = 0, n10 = 0, n11 = 0;
    foreach (Interval *intv, intervals) {
        QString label1 = (attributeID_1.isEmpty()) ? intv->text() : intv->attribute(attributeID_1).toString();
        QString label2 = (attributeID_2.isEmpty()) ? intv->text() : intv->attribute(attributeID_2).toString();
        QString groupByAnnotator1 = groupNameForLabel(label1);
        QString groupByAnnotator2 = groupNameForLabel(label2);
        if (groupByAnnotator1.isEmpty() || groupByAnnotator2.isEmpty()) continue;
        //                   annotator1
        //                   group1     group2
        // annotator2 group1   n00       n01
        //            group2   n10       n11
        if      ((groupByAnnotator1 == groupName_1) && (groupByAnnotator2 == groupName_1)) n00++;
        else if ((groupByAnnotator1 == groupName_2) && (groupByAnnotator2 == groupName_1)) n01++;
        else if ((groupByAnnotator1 == groupName_1) && (groupByAnnotator2 == groupName_2)) n10++;
        else if ((groupByAnnotator1 == groupName_2) && (groupByAnnotator2 == groupName_2)) n11++;
    }
    double Pa = (double(n00 + n11)) / (double(n00+n01+n10+n11));
    double Pe = (double((n00+n10)*(n00+n01)+(n10+n11)*(n01+n11))) / (double(n00+n01+n10+n11)) / (double(n00+n01+n10+n11));
    double k = (Pa - Pe) / (1 - Pe);
    return k;
}

