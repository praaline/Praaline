#include <QDebug>
#include <QString>
#include <QList>
#include <QStringList>
#include <QPointer>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "pnlib/asr/phonetiser/DictionaryPhonetiser.h"
#include "pnlib/asr/phonetiser/PhonemeTranslation.h"
#include "pnlib/asr/sphinx/SphinxPronunciationDictionary.h"
using namespace Praaline::ASR;

#include "pncore/statistics/WordAlign.h"

#include "PFCPhonetiser.h"

struct PFCPhonetiserData {
    DictionaryPhonetiser phonetiser;
    QStringList errorsLiaison;
    QStringList errorsSchwa;
    QStringList liaisonConsonants;
};

PFCPhonetiser::PFCPhonetiser() : d(new PFCPhonetiserData())
{
    d->phonetiser.setSymbolForFalseStarts("/");
    d->liaisonConsonants << "t" << "z" << "n" << "R";
}

PFCPhonetiser::~PFCPhonetiser()
{
    delete d;
}

QString PFCPhonetiser::loadPhonetisationDictionary()
{
    d->phonetiser.readPhonemeTranslation("/mnt/hgfs/DATA/PFCALIGN/phonetisation/sphinx_to_sampa.json");
    d->phonetiser.readPhoneticDictionary("/mnt/hgfs/DATA/PFCALIGN/phonetisation/fr.dict");
    // writePhonemeList("/mnt/hgfs/DATA/PFCALIGN/phonetisation/fr.phonemes");
    return "Phonetisation dictionary loaded";
}

void PFCPhonetiser::reset()
{
    d->phonetiser.clearState();
    d->errorsLiaison.clear();
    d->errorsSchwa.clear();
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
                QString phonetisation_dic = convertPhonetisationsToStars(d->phonetiser.phonetiseToken(word));
                tok_min->setAttribute("phonetisation", phonetisation_dic);
                // Before anything else, save dictionary phonetisations for analysis
                tok_min->setAttribute("phonetisation_dic", phonetisation_dic);
                // Apply PFC coding (except for false starts)
                if (word.endsWith("/")) continue;
                applyPFCLiaison(tok_min);
                applyPFCSchwa(tok_min);
            }
            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
        }
        qDeleteAll(tiersAll);
    }
    return com->ID();
}

QString PFCPhonetiser::writeListOfWordsOOV(const QString &filename)
{
    d->phonetiser.writeTokensOutOfVocabulary(filename);
    return "Output list of OOV words: OK";
}

QString PFCPhonetiser::writeListOfWordsFalseStarts(const QString &filename)
{
    d->phonetiser.writeTokensFalseStarts(filename);
    return "Output list of false starts: OK";
}

QString PFCPhonetiser::convertPhonetisationsToStars(const QString &phonetisations)
{
    QStringList consolidated = phonetisations.split(" | ");
    std::sort(consolidated.begin(), consolidated.end());
    int i = consolidated.count() - 1;
    while (i >= 0) {
        int j = i - 1;
        while (j >= 0) {
            QString phonetisationA = consolidated.at(i).trimmed();
            QString phonetisationB = consolidated.at(j).trimmed();
            WordAlign wa;
            wa.align(phonetisationA.split(" ", QString::SkipEmptyParts), phonetisationB.split(" ", QString::SkipEmptyParts));
            if (((wa.deletionsCount() + wa.insertionsCount()) == 1) && wa.substitutionsCount() == 0) {
                QString shorthand;
                foreach (WordAlign::AlignmentItem item, wa.alignment()) {
                    if      (item.op == "INS") shorthand.prepend(QString(item.hyp).append("* "));
                    else if (item.op == "DEL") shorthand.prepend(QString(item.ref).append("* "));
                    else                       shorthand.prepend(QString(item.hyp).append(" "));
                }
                if (phonetisationA.length() > phonetisationB.length()) {
                    consolidated[i] = shorthand.trimmed();
                    consolidated.removeAt(j);
                    i = consolidated.count() - 1;
                    j = i - 1;
                }
                else if (phonetisationB.length() > phonetisationA.length()) {
                    consolidated[j] = shorthand.trimmed();
                    consolidated.removeAt(i);
                    i = consolidated.count() - 1;
                    j = i - 1;
                }
            }
            else {
                j--;
            }
        }
        i--;
    }
    return consolidated.join(" | ");
}

void PFCPhonetiser::applyPFCLiaison(Praaline::Core::Interval *tok_min)
{
    // F1: 1 = one syllable, 2 = two syllables or more
    // F2: 0 = absence of liaison, 1 = forward linked liaison, 2 = liaison consonant present but not forward linked
    //     3 = uncertainty, 4 = epenthetic liaison
    // F3: consonant
    // F4: context

    bool liaisonPresent(false);
    QString consonant;
    QString token = tok_min->text();
    QString liaison = tok_min->attribute("liaison").toString();
    if (token == liaison) {
        liaisonPresent = false;
    }
    else {
        QString code = QString(liaison).remove(0, token.length());
        // Check if liaison code is well-formed
        if (!code.contains("1") && !code.contains("2")) {
            d->errorsLiaison << QString(token).append("\t").append(liaison).append("\t").append(code);
            return;
        }
        // Decode liaison code
        int nsyll = code.mid(0, 1).toInt();
        int realisation = (code.length() > 1) ? code.mid(1, 1).toInt() : 0;
        consonant = (code.length() > 2) ? code.mid(2, 1) : "";
        QString context = (code.length() > 3) ? code.mid(3, -1) : "";
        // Save liaison attributes
        tok_min->setAttribute("liaison_nsyll", nsyll);
        tok_min->setAttribute("liaison_realisation", realisation);
        tok_min->setAttribute("liaison_consonant", consonant);
        tok_min->setAttribute("liaison_context", context);
        if ((realisation > 0) && (!consonant.isEmpty())) liaisonPresent = true;
    }
    // Process phonetisations
    QStringList phonetisations = tok_min->attribute("phonetisation").toString().split(" | ");
    QStringList phonetisationsFiltered;
    if (liaisonPresent) {
        // With liaison
        foreach (QString phonetisation, phonetisations) {
            QStringList phonemes = phonetisation.split(" ", QString::SkipEmptyParts);
            if (phonemes.isEmpty()) continue;
            if (phonemes.last().endsWith(consonant + "*")) {
                phonemes[phonemes.count() - 1] = phonemes.last().remove("*");
                phonetisationsFiltered << phonemes.join(" ");
            }
            else
                phonetisationsFiltered << phonetisation;
        }
    }
    else {
        // No liaison
        foreach (QString phonetisation, phonetisations) {
            QStringList phonemes = phonetisation.split(" ", QString::SkipEmptyParts);
            if (phonemes.isEmpty()) continue;
            if (phonemes.last().endsWith("*") && !phonemes.last().endsWith("@*")) {
                phonemes.takeLast();
                phonetisationsFiltered << phonemes.join(" ");
            }
            else
                phonetisationsFiltered << phonetisation;
        }
    }
    if (!phonetisationsFiltered.isEmpty())
        tok_min->setAttribute("phonetisation", phonetisationsFiltered.join(" | "));
}


void PFCPhonetiser::applyPFCSchwa(Interval *tok_min)
{
    // F1: Presence/Absence
    // F2: Position of the schwa within the word (monosyllable, initial syllable within polysyllabic word,
    //     internal syllable, final syllable)
    // F3: Left context (C, V)
    // F4: Right context (C, V)

    QString token = tok_min->text();
    QString schwa = tok_min->attribute("schwa").toString();
    QList<bool> schwaPresent;
    // Decode schwa annotation
    QRegularExpression reSchwa("[0-9]+");
    QRegularExpressionMatchIterator i = reSchwa.globalMatch(schwa);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch()) {
            QString code = match.captured(0);
            if (token == code) continue; // skip numbers!
            if (code.left(1) == "0") schwaPresent << false; else schwaPresent << true;
        }
    }
    QStringList phonetisations = tok_min->attribute("phonetisation").toString().split(" | ");
    QStringList phonetisationsFiltered;
    foreach (QString phonetisation, phonetisations) {
        QStringList phonemes = phonetisation.split(" ", QString::SkipEmptyParts);
        if (phonemes.isEmpty()) continue;
        int numberOptionalSchwas(0);
        foreach (QString phoneme, phonemes) { if (phoneme == "@*") numberOptionalSchwas++; }
        if (numberOptionalSchwas == schwaPresent.count()) {
            int i(0);
            QStringList phonemesAdapted;
            foreach (QString phoneme, phonemes) {
                if (phoneme == "@*") {
                    if (schwaPresent.at(i)) phonemesAdapted << "@";
                    i++;
                } else {
                    phonemesAdapted << phoneme;
                }
            }
            phonetisationsFiltered << phonemesAdapted.join(" ");
        } else {
            phonetisationsFiltered << phonetisation;
        }
    }
    if (!phonetisationsFiltered.isEmpty())
        tok_min->setAttribute("phonetisation", phonetisationsFiltered.join(" | "));
}



