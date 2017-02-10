#include "pncore/annotation/Interval.h"
using namespace Praaline::Core;

#include "InterraterAgreement.h"

struct InterraterAgreementData {
    QString classForEmpty;
    QHash<QString, QStringList> classes;
    QHash<QString, QString> classification;
    // A list of annotators (columns): each containing the count for a given category
    QMap<QString, QHash<QString, int> > counts;
};

InterraterAgreement::InterraterAgreement() :
    d(new InterraterAgreementData)
{
}

InterraterAgreement::~InterraterAgreement()
{
    delete d;
}

void InterraterAgreement::addClass(const QString &className, const QStringList &labels, bool includeEmpty)
{
    if (d->classes.contains(className)) removeClass(className);
    d->classes.insert(className, labels);
    foreach (QString label, labels) {
        d->classification.insert(label, className);
    }
    if (includeEmpty) d->classForEmpty = className;
    d->counts.clear();
}

void InterraterAgreement::removeClass(const QString &className)
{
    d->classes.remove(className);
    foreach (QString label, d->classification.keys()) {
        if (d->classification.value(label) == className) {
            d->classification.remove(label);
        }
    }
    if (d->classForEmpty == className) d->classForEmpty = "";
    d->counts.clear();
}

void InterraterAgreement::reset()
{
    d->classes.clear();
    d->classification.clear();
    d->classForEmpty = "";
    d->counts.clear();
}

void InterraterAgreement::resetCounts()
{
    d->counts.clear();
}

QString InterraterAgreement::classify(const QString &label) const
{
    QString className;
    if ((label.isEmpty()) && (!d->classForEmpty.isEmpty())) {
        className = d->classForEmpty;
    }
    else if (d->classification.contains(label)) {
        className = d->classification.value(label);
    }
    return className;
}

void InterraterAgreement::categorise(const QString &annotatorID, const QList<Interval *> &intervals, const QString &attributeID)
{
    QHash<QString, int> count;
    foreach (Interval *intv, intervals) {
        QString label = (attributeID.isEmpty()) ? intv->text() : intv->attribute(attributeID).toString();
        QString className = classify(label);
        if (className.isEmpty())
            continue; // ignore
        else {
            if (count.contains(className))
                count[className] = count.value(className) + 1;
            else
                count.insert(className, 1);
        }
    }
    d->counts.insert(annotatorID, count);
}

double InterraterAgreement::getCohenKappa(const QString &className_1, const QString &className_2,
                                          const QList<Interval *> &intervals,
                                          const QString &attributeID_1, const QString &attributeID_2) const
{
    int n00 = 0, n01 = 0, n10 = 0, n11 = 0;
    foreach (Interval *intv, intervals) {
        QString label1 = (attributeID_1.isEmpty()) ? intv->text() : intv->attribute(attributeID_1).toString();
        QString label2 = (attributeID_2.isEmpty()) ? intv->text() : intv->attribute(attributeID_2).toString();
        QString classByAnnotator1 = classify(label1);
        QString classByAnnotator2 = classify(label2);
        if (classByAnnotator1.isEmpty() || classByAnnotator2.isEmpty()) continue;
        //                   annotator1
        //                   group1     group2
        // annotator2 group1   n00       n01
        //            group2   n10       n11
        if      ((classByAnnotator1 == className_1) && (classByAnnotator2 == className_1)) n00++;
        else if ((classByAnnotator1 == className_2) && (classByAnnotator2 == className_1)) n01++;
        else if ((classByAnnotator1 == className_1) && (classByAnnotator2 == className_2)) n10++;
        else if ((classByAnnotator1 == className_2) && (classByAnnotator2 == className_2)) n11++;
    }
    double Pa = (double(n00 + n11)) / (double(n00+n01+n10+n11));
    double Pe = (double((n00+n10)*(n00+n01)+(n10+n11)*(n01+n11))) / (double(n00+n01+n10+n11)) / (double(n00+n01+n10+n11));
    double k = (Pa - Pe) / (1 - Pe);
    return k;
}

double InterraterAgreement::getFleissKappa() const
{
    return -1.0;
}

QStandardItemModel *InterraterAgreement::createTableModel()
{
    QStandardItemModel *model = new QStandardItemModel();
    model->setRowCount(d->classes.count());
    model->setColumnCount(d->counts.count()); // number of annotators
    model->setHorizontalHeaderLabels(d->counts.keys());
    model->setVerticalHeaderLabels(d->classes.keys());
    int col = 0;
    foreach (QString annotatorID, d->counts.keys()) {
        QHash<QString, int> count = d->counts.value(annotatorID);
        int row = 0;
        foreach (QString className, d->classes.keys()) {
            QStandardItem *item;
            if (count.contains(className)) {
                item = new QStandardItem(QString::number(count.value(className)));
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


