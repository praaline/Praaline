#include <QDebug>
#include <QString>
#include <QList>
#include <QMap>
#include <QHash>
#include <QPointer>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/PointTier.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/SequenceTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "ORFEOReader.h"

ORFEOReader::ORFEOReader()
{

}

QString ORFEOReader::readOrfeoFile(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error: No communication";
    QString filename = com->ID() + ".orfeo";
    QString path;
    // Open ORFEO file
    QFile file(path + filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return "Error opening ORFEO file";
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    ret.append(QString("Communication ID: %1\n").arg(com->ID()));
    // Data structures
    QHash<QString, QList<Interval *> > orfeoData;
    // Process file
    do {
        QString line = stream.readLine();
        if (line.startsWith("#")) continue;
        QStringList fields = line.split("\t");
        if (fields.count() < 13) {
            qDebug() << "Error: line with less than 13 fields";
        }
        // Read ORFEO token
        // int ID = fields.at(0).toInt();
        QString form = fields.at(1);
        QString lemma = fields.at(2);
        QString UPOSTag = fields.at(3);
        QString XPOSTag = fields.at(4);
        QString feats = fields.at(5);
        int head = fields.at(6).toInt();
        QString deprel = fields.at(7);
        QString deps = fields.at(8);
        QString misc = fields.at(9);
        double tMin_s = fields.at(10).toDouble();
        double tMax_s = fields.at(11).toDouble();
        QString speakerID = fields.at(12);
        // Create interval for ORFEO token
        Interval *intv = new Interval(RealTime::fromSeconds(tMin_s), RealTime::fromSeconds(tMax_s), form);
        intv->setAttribute("lemma", lemma);
        intv->setAttribute("UPOS", UPOSTag);
        intv->setAttribute("XPOS", XPOSTag);
        intv->setAttribute("feats", feats);
        intv->setAttribute("head", head);
        intv->setAttribute("deprel", deprel);
        intv->setAttribute("deps", deps);
        intv->setAttribute("misc", misc);
        // Include token in the list of tokens of the appropriate speaker
        orfeoData[speakerID].append(intv);
    } while (!stream.atEnd());
    file.close();
    // Create tiers and save them into the database
    foreach (QString speakerID, orfeoData.keys()) {
        IntervalTier *tier = new IntervalTier("orfeo_token", orfeoData.value(speakerID));
        com->repository()->annotations()->saveTier(com->ID(), speakerID, tier);
        ret.append(QString("\tSpeaker ID: %1\t saved %2 tokens\n").arg(speakerID).arg(orfeoData.value(speakerID).count()));
    }

    return ret;
}
