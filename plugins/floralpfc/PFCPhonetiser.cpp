#include <QString>
#include <QList>
#include <QPointer>
#include <QMap>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "pnlib/asr/phonetiser/PhonemeTranslation.h"
#include "pnlib/asr/sphinx/SphinxPronunciationDictionary.h"
using namespace Praaline::ASR;

#include "PFCPhonetiser.h"

struct PFCPhonetiserData {
    SphinxPronunciationDictionary *sphinxDictionary;
    PhonemeTranslation phonemeTranslation;
    QString symbolFalseStart;
    QStringList wordsFalseStarts;
    QStringList wordsOOV;
};

PFCPhonetiser::PFCPhonetiser() : d(new PFCPhonetiserData())
{
    d->symbolFalseStart = "/";
    d->sphinxDictionary = new SphinxPronunciationDictionary();
}

PFCPhonetiser::~PFCPhonetiser()
{
    delete d->sphinxDictionary;
    delete d;
}

QString PFCPhonetiser::loadPhonetisationDictionary()
{
    d->phonemeTranslation.read("/mnt/hgfs/DATA/PFCALIGN/phonetisation/sphinx_to_sampa.json");
    d->sphinxDictionary->readFromFile("/mnt/hgfs/DATA/PFCALIGN/phonetisation/fr.dict");
    d->sphinxDictionary->writePhonemeList("/mnt/hgfs/DATA/PFCALIGN/phonetisation/fr.phonemes");
    return "Phonetisation dictionary loaded";
}

QString PFCPhonetiser::convertSphinxToSampa(const QString &sphinx)
{
    QStringList phonetisationsSampa;
    foreach (QString phonetisationSphinx, sphinx.split("|")) {
        QString phonetisationSampa;
        foreach (QString phoneSphinx, phonetisationSphinx.split(" ", QString::SkipEmptyParts)) {
            QString phoneSampa = d->phonemeTranslation.translate(phoneSphinx);
            phonetisationSampa.append(phoneSampa).append(" ");
        }
        phonetisationsSampa << phonetisationSampa.trimmed();
    }
    return phonetisationsSampa.join(" | ");
}

QString PFCPhonetiser::phonetiseFromDictionary(QPointer<CorpusCommunication> com)
{
    if (!com) return "No Communication";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "tok_min");
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;
            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                if (tok_min->isPauseSilent()) {
                    tok_min->setAttribute("phonetisation", "_");
                    continue;
                }
                QString word = tok_min->text().toLower();
                if (word.startsWith("(")) continue;
                if (word.contains(" ")) word.replace(" ", "_");
                QString phonetisation = convertSphinxToSampa(d->sphinxDictionary->phonetise(word));
                if (phonetisation.isEmpty()) {
                    if (word.endsWith(d->symbolFalseStart)) {
                        if (!d->wordsFalseStarts.contains(word)) d->wordsFalseStarts << word;
                    } else {
                        if (!d->wordsOOV.contains(word)) d->wordsOOV << word;
                    }
                }
                else
                    tok_min->setAttribute("phonetisation", phonetisation);
            }
            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
        }
        qDeleteAll(tiersAll);
    }
    return com->ID();
}

QString PFCPhonetiser::writeListOfWordsOOV(const QString &filename)
{
    QFile fileOut(filename);
    if ( !fileOut.open( QIODevice::WriteOnly | QIODevice::Text ) ) return "Output list of OOV words: Error opening file";
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    foreach (QString word, d->wordsOOV) {
        out << word << "\n";
    }
    fileOut.close();
    return "Output list of OOV words: OK";
}

QString PFCPhonetiser::writeListOfWordsFalseStarts(const QString &filename)
{
    QFile fileOut(filename);
    if ( !fileOut.open( QIODevice::WriteOnly | QIODevice::Text ) ) return "Output list of false starts: Error opening file";
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    foreach (QString word, d->wordsFalseStarts) {
        out << word << "\n";
    }
    fileOut.close();
    return "Output list of false starts: OK";
}

QString PFCPhonetiser::applyPFCLiaison(QPointer<CorpusCommunication> com)
{

    return QString();
}

QString PFCPhonetiser::applyPFCSchwa(QPointer<CorpusCommunication> com)
{

    return QString();
}
