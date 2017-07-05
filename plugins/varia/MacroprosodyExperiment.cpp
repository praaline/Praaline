#include <QDebug>
#include <QString>
#include <QList>
#include <QMap>
#include "pncore/statistics/StatisticalSummary.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/PointTier.h"
#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "MacroprosodyExperiment.h"

void MacroprosodyExperiment::calculateZScoreForJoystickDataPerParticipant(const QList<QPointer<CorpusCommunication> > &communications, const QString &tierName)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    QHash<QString, QList<double> > values;
    QHash<QString, StatisticalSummary> statistics;

    qDebug() << "Collecting data";
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                PointTier *tier_joystick = tiers->getPointTierByName(tierName);
                if (!tier_joystick) continue;
                QList<double> theseValues;
                foreach (Point *p, tier_joystick->points()) theseValues << p->attribute("axis1").toDouble();
                if (values.contains(speakerID))
                    values[speakerID].append(theseValues);
                else
                    values.insert(speakerID, theseValues);
            }
            qDeleteAll(tiersAll);
        }
        qDebug() << "   " << com->ID();
    }
    qDebug() << "Calculating statistics";
    foreach (QString speakerID, values.keys()) {
        StatisticalSummary stat(values[speakerID]);
        statistics.insert(speakerID, stat);
    }
    qDebug() << "Updating normalised values";
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                PointTier *tier_joystick = tiers->getPointTierByName(tierName);
                if (!tier_joystick) continue;
                foreach (Point *p, tier_joystick->points()) {
                    double normalised = (p->attribute("axis1").toDouble() - statistics[speakerID].mean()) / statistics[speakerID].stDev();
                    p->setAttribute("value_norm", normalised);
                }
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_joystick);
            }
            qDeleteAll(tiersAll);
        }
        qDebug() << "   " << com->ID();
    }
}

void MacroprosodyExperiment::calculateZScoreForJoystickDataPerSampleAndParticipant(const QList<QPointer<CorpusCommunication> > &communications, const QString &tierName)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                PointTier *tier_joystick = tiers->getPointTierByName(tierName);
                if (!tier_joystick) continue;
                QList<double> values;
                // Collect
                foreach (Point *p, tier_joystick->points()) values << p->attribute("axis1").toDouble();
                // Calculate stats
                StatisticalSummary stat(values);
                // Normalise
                foreach (Point *p, tier_joystick->points()) {
                    // double normalised = (p->attribute("axis1").toDouble() - stat.mean()) / stat.stDev();
                    double normalised = (p->attribute("axis1").toDouble()) / stat.stDev();
                    p->setAttribute("value_norm", normalised);
                }
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_joystick);
            }
            qDeleteAll(tiersAll);
        }
        qDebug() << "   " << com->ID();
    }
}

void MacroprosodyExperiment::createCombinedJoystickData(const QList<QPointer<CorpusCommunication> > &communications, const QString &tierName)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            QList<RealTime> times;
            QList<QList<double> > data;
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                PointTier *tier_joystick = tiers->getPointTierByName(tierName);
                if (!tier_joystick) continue;
                for (int i = 0; i < tier_joystick->count(); ++i) {
                    if (i >= times.count()){
                        times.append(tier_joystick->point(i)->time());
                    }
                    if (i < data.count()) {
                        data[i].append(tier_joystick->point(i)->attribute("value_norm").toDouble());
                    } else {
                        QList<double> values;
                        values.append(tier_joystick->point(i)->attribute("value_norm").toDouble());
                        data.append(values);
                    }
                }
            }
            QList<Point *> combinedPoints;
            for (int i = 0; (i < times.count()) && (i < data.count()); ++i) {
                Point * p = new Point(times.at(i), "");
                StatisticalSummary stat(data.at(i));
                p->setAttribute("value_mean", stat.mean());
                p->setAttribute("value_stdev", stat.stDev());
                combinedPoints << p;
            }
            PointTier *tier_combined = new PointTier(tierName + "_combined", combinedPoints);
            com->repository()->annotations()->saveTier(annotationID, "combined", tier_combined);
            qDeleteAll(tiersAll);
        }
        qDebug() << "   " << com->ID();
    }
}
