#include <QDebug>
#include <QString>
#include <QList>
#include <QMap>
#include <QFile>
#include <QTextStream>

#include "pncore/annotation/IntervalTier.h"
#include "pncore/corpus/Corpus.h"
#include "pnlib/asr/sphinx/SphinxAcousticModelTrainer.h"
#include "pnlib/asr/sphinx/SphinxPronunciationDictionary.h"

#include "BroadClassAligner.h"

struct BroadClassAlignerData
{
    QString defaultBroadClass;
    QList<QString> broadPhoneticClasses;
    QHash<QString, QString> correspondances;
    SphinxPronunciationDictionary dictionary;
};

BroadClassAligner::BroadClassAligner() :
    d(new BroadClassAlignerData)
{
    d->defaultBroadClass = "VV";
}

BroadClassAligner::~BroadClassAligner()
{
    delete d;
}

void BroadClassAligner::addBroadPhoneticClass(const QString &name, const QList<QString> &phonemes)
{
    d->broadPhoneticClasses << name;
    foreach (QString ph, phonemes) {
        d->correspondances.insert(ph, name);
    }
}

void BroadClassAligner::resetClasses()
{
    d->broadPhoneticClasses.clear();
    d->correspondances.clear();
}

void BroadClassAligner::initialiseFR()
{
    resetClasses();
    d->defaultBroadClass = "VV";
    addBroadPhoneticClass("PP", QList<QString>() << "p" << "b" << "t" << "d" << "k" << "g");      // plosives
    addBroadPhoneticClass("FF", QList<QString>() << "f" << "v" << "s" << "z" << "S" << "Z");      // fricatives
    addBroadPhoneticClass("NN", QList<QString>() << "m" << "n");                                  // nasals
    addBroadPhoneticClass("LL", QList<QString>() << "l" << "R");                                  // liquids
    addBroadPhoneticClass("GG", QList<QString>() << "w" << "H" << "j");                           // glides
    addBroadPhoneticClass("VV", QList<QString>() << "i" << "e" << "E" << "a" << "A" <<
                                                    "o" << "O" << "y" << "2" << "9" <<
                                                    "u" << "U~");             // vowels
    addBroadPhoneticClass("VN", QList<QString>() << "e~" << "a~" << "o~" << "9~");                // nasal vowels
    addBroadPhoneticClass("EE", QList<QString>() << "@");                                         // schwa
    addBroadPhoneticClass("SIL", QList<QString>() << "_" << "dummy" << "#");                      // silence
}

void BroadClassAligner::initialiseFRcv()
{
    resetClasses();
    d->defaultBroadClass = "V";
    addBroadPhoneticClass("C", QList<QString>() <<
                          "p" << "b" << "t" << "d" << "k" << "g" << "f" << "v" << "s" << "z" << "S" << "Z" <<
                          "m" << "n" << "l" << "R" << "w" << "H" << "j");
    addBroadPhoneticClass("V", QList<QString>() <<
                          "i" << "e" << "E" << "a" << "A" << "o" << "O" << "y" << "2" << "9" << "u" << "U~" <<
                          "e~" << "a~" << "o~" << "9~" << "@");
    addBroadPhoneticClass("SIL", QList<QString>() << "_" << "dummy" << "#");                      // silence
}

void BroadClassAligner::initialiseFRSphinx()
{
    resetClasses();
    d->defaultBroadClass = "VV";
    addBroadPhoneticClass("PP", QList<QString>() << "pp" << "bb" << "tt" << "dd" << "kk" << "gg");  // plosives
    addBroadPhoneticClass("FF", QList<QString>() << "ff" << "vv" << "ss" << "zz" << "ch" << "jj");  // fricatives
    addBroadPhoneticClass("NN", QList<QString>() << "mm" << "nn" << "gn");                          // nasals
    addBroadPhoneticClass("LL", QList<QString>() << "ll" << "rr");                                  // liquids
    addBroadPhoneticClass("GG", QList<QString>() << "ww" << "uy" << "yy");                          // glides
    addBroadPhoneticClass("VV", QList<QString>() << "ii" << "ei" << "ai" << "aa" << "au" <<
                                                    "oo" << "ou" << "uu" << "eu" << "oe");          // vowels
    addBroadPhoneticClass("VN", QList<QString>() << "in" << "an" << "on" << "un");                  // nasal vowels
    addBroadPhoneticClass("EE", QList<QString>() << "ee");                                          // schwa
    addBroadPhoneticClass("SIL", QList<QString>() << "_");                                          // silence
}


QString BroadClassAligner::phonemeToBroadClass(const QString &phoneme) const
{
    return d->correspondances.value(phoneme.trimmed(), d->defaultBroadClass);
}

bool BroadClassAligner::adaptDictionary(const QString &filenameInput, const QString &filenameOutput) const
{
    QMap<QString, QString> dictionary;

    QFile fileDicIn(filenameInput);
    if ( !fileDicIn.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    QTextStream dicIn(&fileDicIn);
    dicIn.setCodec("UTF-8");
    while (!dicIn.atEnd()) {
        QString line = dicIn.readLine();
        if (!line.contains(" ")) continue;
        QStringList phonetisation = line.section(" ", 1, -1).split(" ");
        QStringList broadphonetisation;
        foreach (QString ph, phonetisation) {
            broadphonetisation << phonemeToBroadClass(ph);
        }
        dictionary.insert(broadphonetisation.join(""), broadphonetisation.join(" "));
    }
    fileDicIn.close();

    // Create broad phonetic class transcription dictionary
    QFile fileDicOut(filenameOutput);
    if ( !fileDicOut.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream dicOut(&fileDicOut);
    dicOut.setCodec("UTF-8");
    foreach (QString word, dictionary.keys()) {
        dicOut << word << " " << dictionary.value(word) << "\n";
    }
    fileDicOut.close();
    return true;
}

void BroadClassAligner::updatePhoneTierWithBroadClasses(IntervalTier *tier, const QString &attributeBroadClass) const
{
    if (!tier) return;
    foreach (Interval *phone, tier->intervals()) {
        QString p = phonemeToBroadClass(phone->text());
        // qDebug() << phone->text() << " " << p;
        phone->setAttribute(attributeBroadClass, p);
    }
}

void BroadClassAligner::updateTokenTierWithBroadClasses(IntervalTier *tier_phone, const QString &attributeBroadClassPhone,
                                                        IntervalTier *tier_token, const QString &attributeBroadClassToken) const
{
    if (!tier_phone) return;
    if (!tier_token) return;
    foreach (Interval *token, tier_token->intervals()) {
        QList<Interval *> phones = tier_phone->getIntervalsContainedIn(token);
        QString bcptranscript, bcphonetisation;
        foreach (Interval *ph, phones) {
            QString p = ph->attribute(attributeBroadClassPhone).toString();
            if (p != "SIL") { bcptranscript.append(p); bcphonetisation.append(p).append(" "); }
        }
        token->setAttribute(attributeBroadClassToken, bcptranscript);
        d->dictionary.addWord(bcptranscript, bcphonetisation);
    }
}

void BroadClassAligner::prepareTiers(QPointer<Corpus> corpus, QList<QPointer<CorpusCommunication> > communications)
{
    foreach (QPointer<CorpusCommunication> com, communications) {
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID());
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_phone = tiers->getIntervalTierByName("cid_phoneme");
                if (!tier_phone) continue;
                updatePhoneTierWithBroadClasses(tier_phone, "broad_phonetic_class");
                IntervalTier *tier_token = tiers->getIntervalTierByName("cid_syll");
                if (!tier_token) continue;
                updateTokenTierWithBroadClasses(tier_phone, "broad_phonetic_class",
                                                tier_token, "broad_phonetic_class");
                tier_token->replaceAttributeText("broad_phonetic_lcass", "SIL", "");
                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_phone);
                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_token);
            }
            qDeleteAll(tiersAll);
        }
    }
}

void BroadClassAligner::prepareBPCTrainingFromCommunications(QPointer<Corpus> corpus, QList<QPointer<CorpusCommunication> > communications,
                                                             const QString &outputPath)
{
    if (!corpus) return;
    QScopedPointer<SphinxAcousticModelTrainer> trainer(new SphinxAcousticModelTrainer);
    QStringList unknownWords;

    initialiseFRcv();
//    BPCA.buildDictionary("/home/george/broad-align/etc/original-french-62k.dic",
//                         "/home/george/broad-align/etc/broad.dic");

    prepareTiers(corpus, communications);

    d->dictionary.writeToFile(outputPath + "/broad.dic");

    trainer->setTierUtterances("cid_transcription", "");
    trainer->setTierTokens("cid_syll", "broad_phonetic_class");
    trainer->setOutputPath(outputPath);
    trainer->setPronunciationDictionary(&(d->dictionary));
    trainer->createFiles(corpus, communications, unknownWords, true, false);
}



