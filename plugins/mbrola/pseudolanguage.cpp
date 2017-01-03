#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QPointer>
#include <QMap>
#include <QDebug>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"

#include "pseudolanguage.h"

PseudoLanguage::PseudoLanguage()
{

}

// "D:/DROPBOX/2015-10_SP8_ProsodicBoundariesExpe/possible_diphones.txt"
bool PseudoLanguage::createListOfPossibleDiphones(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications,
                                                  const QString &filename, int cutOffThreshold)
{
    QFile fileOut(filename);
    if (! fileOut.open(QFile::WriteOnly | QFile::Text)) return false;
    QTextStream out(&fileOut);
    out.setCodec("ISO 8859-1");

    QHash<QString, int> possible_diphones;

    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            if (!rec) continue;
            foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
                if (!annot) continue;

                QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID());
                foreach (QString speakerID, tiersAll.keys()) {
                    QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);

                    IntervalTier *tier_phones = tiers->getIntervalTierByName("phone");
                    if (!tier_phones) continue;

                    for (int i = 0; i < tier_phones->count() - 1; ++i) {
                        QString ph1 = tier_phones->interval(i)->text();
                        QString ph2 = tier_phones->interval(i+1)->text();
                        if (ph1 == "_" || ph2 == "_") continue;
                        QString diphone = QString("%1 %2").arg(ph1).arg(ph2);
                        if (possible_diphones.contains(diphone))
                            possible_diphones[diphone] = possible_diphones[diphone] + 1;
                        else
                            possible_diphones.insert(diphone, 1);
                    }
                }
                qDeleteAll(tiersAll);
            }
        }
    }

    foreach (QString diphone, possible_diphones.keys()) {
        if (possible_diphones.value(diphone) > cutOffThreshold)
            out << diphone << "\t" << possible_diphones.value(diphone) << "\n";
    }
    fileOut.close();
    return true;
}

int randInt(int low, int high)
{
    // returns a random integer value between low and high
    return qrand() % ((high + 1) - low) + low;
}

QString substitutePhoneme(const QString &phoneme, const QString &contextLeft, const QString &contextRight,
                          const QList<QString> &phonemeClass, const QList<QString> &possible_diphones)
{
    int i = 0;
    while (i < 3) {
        QString candidate = phonemeClass.at(randInt(0, phonemeClass.count() - 1));
        QString left = QString("%1 %2").arg(contextLeft).arg(candidate);
        QString right = QString("%1 %2").arg(candidate).arg(contextRight);
        if ((possible_diphones.contains(left) || contextLeft == "_") &&
            (possible_diphones.contains(right) || contextRight == "_")) {

            if (!(contextLeft == "_" && candidate == "A"))
                return candidate;
        }
        i++;
    }
    if (phoneme.trimmed().isEmpty()) return "_";
    return phoneme;
}

void PseudoLanguage::substitutePhonemes(IntervalTier *tier_phones, const QString &attributeID)
{
    QList<QString> plosives, fricatives, nasals, liquids, glides, vowels, vowels_nasal, allPhonemes;
    plosives << "p" << "b" << "t" << "d" << "k" << "g";
    fricatives << "f" << "v" << "s" << "z" << "S" << "Z";
    nasals << "m" << "n";
    liquids << "l" << "R";
    glides << "w" << "H" << "j";
    vowels << "i" << "e" << "E" << "a" << "A" << "o" << "O" << "y" << "2" << "9";
    vowels_nasal << "e~" << "a~" << "o~" << "9~";
    QList<QList<QString> > phonemeClasses;
    phonemeClasses << plosives << fricatives << nasals << liquids << vowels << vowels_nasal;
    allPhonemes << plosives << fricatives << nasals << liquids << glides << vowels << vowels_nasal << "_";

    // Read possible diphones
    QList<QString> possible_diphones;
    QFile file("D:/DROPBOX/2015-10_SP8_ProsodicBoundariesExpe/possible_diphones.txt");
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return;
    QTextStream stream(&file);
    do {
        QString line = stream.readLine();
        if (line.isEmpty()) continue;
        possible_diphones << line.section("\t", 0, 0);
    } while (!stream.atEnd());
    file.close();

    // Substitute phonemes
    for (int i = 1; i < tier_phones->count() - 1; ++i) {
        QString phoneme = tier_phones->interval(i)->text();
        if (phoneme.isEmpty()) phoneme = "_";
        if (!allPhonemes.contains(phoneme))
            phoneme = allPhonemes.at(randInt(0, allPhonemes.count() - 1));

        QString contextLeft = tier_phones->interval(i - 1)->text();
        QString contextRight = tier_phones->interval(i + 1)->text();
        QString substitute = phoneme;
        foreach(QList<QString> phonemeClass, phonemeClasses) {
            if (phonemeClass.contains(phoneme)) {
                substitute = substitutePhoneme(phoneme, contextLeft, contextRight, phonemeClass, possible_diphones);
                break;
            }
        }
        if (substitute.isEmpty()) substitute = "_";
        tier_phones->interval(i)->setAttribute(attributeID, substitute);
    }
}




