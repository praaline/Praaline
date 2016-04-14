#ifndef BROADCLASSALIGNER_H
#define BROADCLASSALIGNER_H

#include <QString>
#include <QList>
#include <QHash>

class IntervalTier;

class BroadClassAligner
{
public:
    BroadClassAligner();

    void addBroadPhoneticClass(const QString &name, const QList<QString> &phonemes);
    void resetClasses();
    void initialiseFR();
    void initialiseCV();

    QString phonemeToBroadClass(const QString &phoneme) const;
    void updatePhoneTierWithBroadClasses(IntervalTier *tier, const QString &attributeBroadClass) const;

private:
    QString m_defaultBroadClass;
    QList<QString> m_broadPhoneticClasses;
    QHash<QString, QString> m_correspondances;
};

#endif // BROADCLASSALIGNER_H
