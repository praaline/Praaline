#ifndef INTERRATERAGREEMENT_H
#define INTERRATERAGREEMENT_H

#include <QString>
#include <QList>
#include <QPair>
#include <QHash>
#include <QMap>
#include <QStandardItemModel>

namespace Praaline {
namespace Core {
class Interval;
}
}
using namespace Praaline::Core;

struct InterraterAgreementData;

class InterraterAgreement
{
public:
    InterraterAgreement();
    ~InterraterAgreement();

    void addClass(const QString &className, const QStringList &labels, bool includeEmpty = false);
    void removeClass(const QString &className);
    QString classify(const QString &label) const;

    void reset();
    void resetCounts();
    void categorise(const QString &annotatorID, const QList<Interval *> &intervals, const QString &attributeID = QString());
    QStandardItemModel *createTableModel();

    double getCohenKappa(const QString &className_1, const QString &className_2,
                         const QList<Interval *> &intervals,
                         const QString &attributeID_1 = QString(), const QString &attributeID_2 = QString()) const;
    double getFleissKappa() const;

private:
    InterraterAgreementData *d;
};

#endif // INTERRATERAGREEMENT_H
