#include <QString>
#include <QPointer>
#include <QList>
#include <QMap>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/SequenceTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "SentencesSplitter.h"

struct SentencesSplitterData {
    // Sentences {AnnotationID, SpeakerID, sentences}
    QMap<QString, QMap<QString, QStringList> > sentences;
};

SentencesSplitter::SentencesSplitter() :
    d(new SentencesSplitterData())
{
}

SentencesSplitter::~SentencesSplitter()
{
    delete d;
}

QString SentencesSplitter::exportSentences(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;

            IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");
            if (!tier_tok_mwu) continue;
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;

            ret.append(annotationID).append("\t").append(speakerID).append("\t");
            foreach (Interval *tok_mwu, tier_tok_mwu->intervals()) {
                if (tok_mwu->isPauseSilent()) continue;
                if (tok_mwu->attribute("discourse") == "*") {
                    ret.append(tok_mwu->text()).append("\n").append(annotationID).append("\t").append(speakerID).append("\t");
                } else {
                    ret.append(tok_mwu->text()).append(" ");
                }
            }
            ret = ret.trimmed().append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

QString SentencesSplitter::readBreaksFile(const QString &filename)
{
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        return QString("Error reading file %1").arg(filename);
    QString line;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    d->sentences.clear();
    QString annotationID, speakerID;
    do {
        line = stream.readLine().trimmed();
        if (line.startsWith("#") && line.contains("\t")) {
            annotationID = line.section("\t", 0, 0).remove("#").trimmed();
            speakerID = line.section("\t", 1, 1).trimmed();
        }
        else if (!line.isEmpty()) {
            if (!d->sentences.contains(annotationID))
                d->sentences.insert(annotationID, QMap<QString, QStringList>());
            if (!d->sentences[annotationID].contains(speakerID))
                d->sentences[annotationID].insert(speakerID, QStringList());
            d->sentences[annotationID][speakerID].append(line.trimmed());
        }
    } while (!stream.atEnd());
    file.close();
    return QString("Read sentences file");
}

QString SentencesSplitter::importBreaks(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}
