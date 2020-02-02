#include <QDebug>
#include <QString>
#include <QList>
#include <QStack>
#include <QMap>
#include <QHash>
#include <QPointer>
#include <QTextStream>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/SequenceTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
using namespace Praaline::Core;

#include "SequencerDisfluencies.h"

struct SequencerDisfluenciesData {
    // Lists of valid disfluency codes
    QStringList codesSkipped;
    QStringList codesSingleToken;
    QStringList codesRepetition;
    QStringList codesStructured;
    QStringList codesComplex;
    // Where are disfluency codes stored?
    QString annotationLevel;
    QString annotationAttribute;
    // Sequences will be stored on this level
    QString sequencesLevel;
};

SequencerDisfluencies::SequencerDisfluencies() :
    d(new SequencerDisfluenciesData())
{
    d->codesSkipped << "SIL" << "PARA";
    d->codesSingleToken << "FIL" << "FST" << "LEN" << "WDP" << "MSP";
    d->codesRepetition << "REP" << "RED";
    d->codesStructured << "DEL" << "SUB" << "INS";
    d->codesComplex << "COM";
    // For stuttered speech
    d->codesSingleToken << "BLOC" << "BLEN";
    d->codesRepetition << "BREP" << "BRED";
    d->codesComplex << "BCOM";
    // Levels and attributes
    d->annotationLevel = "tok_min";
    d->annotationAttribute = "disfluency";
    d->sequencesLevel = "disfluencies";
}

SequencerDisfluencies::~SequencerDisfluencies()
{
    delete d;
}

QStringList SequencerDisfluencies::codesSkipped() const
{
    return d->codesSkipped;
}

void SequencerDisfluencies::setCodesSkipped(const QStringList &codes)
{
    d->codesSkipped = codes;
}

QStringList SequencerDisfluencies::codesSingleToken() const
{
    return d->codesSingleToken;
}

void SequencerDisfluencies::setCodesSingleToken(const QStringList &codes)
{
    d->codesSingleToken = codes;
}

QStringList SequencerDisfluencies::codesRepetitions() const
{
    return d->codesRepetition;
}

void SequencerDisfluencies::setCodesRepetitions(const QStringList &codes)
{
    d->codesRepetition = codes;
}

QStringList SequencerDisfluencies::codesStructured() const
{
    return d->codesStructured;
}

void SequencerDisfluencies::setCodesStructured(const QStringList &codes)
{
    d->codesStructured = codes;
}

QStringList SequencerDisfluencies::codesComplex() const
{
    return d->codesComplex;
}

void SequencerDisfluencies::setCodesComplex(const QStringList &codes)
{
    d->codesComplex = codes;
}

QString SequencerDisfluencies::annotationLevel() const
{
    return d->annotationLevel;
}

void SequencerDisfluencies::setAnnotationLevel(const QString &levelID)
{
    d->annotationLevel = levelID;
}

QString SequencerDisfluencies::annotationAttribute() const
{
    return d->annotationAttribute;
}

void SequencerDisfluencies::setAnnotationAttribute(const QString &annotationID)
{
    d->annotationAttribute = annotationID;
}

QString SequencerDisfluencies::sequencesLevel() const
{
    return d->sequencesLevel;
}

void SequencerDisfluencies::setSequencesLevel(const QString &levelID)
{
    d->sequencesLevel = levelID;
}

QString SequencerDisfluencies::getAllDistinctSequences(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    QHash<QString, int> sequences;
    if (!com) return ret;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->annotationLevel);
            if (!tier_tokens) continue;

            QString currentSequence;
            for (int i = 0; i < tier_tokens->count(); ++i) {
                QString dis = tier_tokens->at(i)->attribute(d->annotationAttribute).toString().trimmed();
                dis = dis.replace("SIL:b", "SIL").replace("SIL:l", "SIL");

                if (dis.isEmpty() && !currentSequence.isEmpty()) {
                    if (sequences.contains(currentSequence)) {
                        sequences[currentSequence] = sequences[currentSequence] + 1;
                    } else {
                        sequences.insert(currentSequence, 1);
                    }
                    currentSequence.clear();
                }
                else if (!dis.isEmpty()) {
                    currentSequence.append(" ").append(dis);
                }
            }
        }
        qDeleteAll(tiersAll);
    }

    foreach (QString sequence, sequences.keys()) {
        ret.append(QString("%1\t%2\t%3\n").arg(com->ID()).arg(sequence).arg(sequences.value(sequence)));
    }
    return ret.trimmed();
}

void SequencerDisfluencies::createAttribute(CorpusRepository *repository, AnnotationStructureLevel *level, const QString &prefix,
                                            const QString &ID, const QString &name /*= QString()*/, const QString &description /*= QString()*/,
                                            const DataType &datatype /*= DataType(DataType::VarChar, 256)*/, int order /*= 0*/,
                                            bool indexed /*= false*/, const QString &nameValueList /*= QString()*/)
{
    if (level->hasAttribute(ID)) return;
    AnnotationStructureAttribute *attr = new AnnotationStructureAttribute(prefix + ID, name, description, datatype,
                                                                          order, indexed, nameValueList);
    if (repository->annotations()->createAnnotationAttribute(level->ID(), attr))
        level->addAttribute(attr);
}

void SequencerDisfluencies::createDisfluencySequenceAnnotationLevel(CorpusRepository *repository)
{
    if (!repository) return;
    // If need be, create the annotation level for disfluency sequences
    AnnotationStructureLevel *level_seq = repository->annotationStructure()->level(d->sequencesLevel);
    if (!level_seq) {
        level_seq = new AnnotationStructureLevel(d->sequencesLevel, AnnotationStructureLevel::SequencesLevel, "Disfluency Sequences", "Automatically generated from token annotation");
        if (!repository->annotations()->createAnnotationLevel(level_seq)) return;
        repository->annotationStructure()->addLevel(level_seq);
    }
    // Create disfluency sequence attributes where necessary
    createAttribute(repository, level_seq, "", "textSequence", "Disfluency Sequence Text", "");
    createAttribute(repository, level_seq, "", "textReparandum", "Reparandum Text", "");
    createAttribute(repository, level_seq, "", "textInterregnum", "Interregnum Text", "");
    createAttribute(repository, level_seq, "", "textReparans", "Reparans Text", "");
    createAttribute(repository, level_seq, "", "textSequenceAnnotated", "Disfluency sequence with annotation tags", "");
    createAttribute(repository, level_seq, "", "indexInterruptionPoint", "Index of Interruption Point start", "", DataType::Integer);
    createAttribute(repository, level_seq, "", "indexReparans", "Index of Reparans start", "", DataType::Integer);
}

QString SequencerDisfluencies::checkAnnotation(Praaline::Core::CorpusCommunication *com)
{
    bool createSequences(true);
    QString ret;
    if (!com) return ret;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->annotationLevel);
            if (!tier_tokens) continue;

            StructuredDisfluencyState repetitionState = Outside; QString repetitionCode;
            StructuredDisfluencyState structuredState = Outside; QString structuredCode;
            bool insideComplexDisfluency = false;                QString complexCode;
            int repetitionStart(-1), repetitionInterruptionStart(-1), repetitionReparansStart(-1);
            int structuredStart(-1), structuredInterruptionStart(-1), structuredReparansStart(-1);
            int complexStart(-1);

            QList<Sequence *> sequences;

            QStringList errors;
            for (int i = 0; i < tier_tokens->count(); ++i) {
                bool hasError(false);
                QString errorMessage;
                QString disAll = tier_tokens->at(i)->attribute(d->annotationAttribute).toString().trimmed();
                disAll = disAll.replace("+SIL:b", "").replace("+SIL:l", "").replace("+SIL", "")
                               .replace("SIL:b", "").replace("SIL:l", "").replace("SIL", "");
                if ((repetitionState == Reparans) && !(disAll.split("+").contains(repetitionCode + "_"))) {
                    if (createSequences) {
                        Sequence *seq = new Sequence(repetitionStart, i - 1, repetitionCode);
                        seq->setAttribute("indexInterruptionPoint", repetitionInterruptionStart);
                        seq->setAttribute("indexReparans", repetitionReparansStart);
                        sequences << seq;
                    }
                    repetitionState = Outside; repetitionCode.clear();
                    repetitionStart = -1; repetitionInterruptionStart = -1; repetitionReparansStart = -1;
                }
                if ((structuredState == Reparans) && !(disAll.split("+").contains(structuredCode + "_"))) {
                    if (createSequences) {
                        Sequence *seq = new Sequence(structuredStart, i - 1, structuredCode);
                        seq->setAttribute("indexInterruptionPoint", structuredInterruptionStart);
                        seq->setAttribute("indexReparans", structuredReparansStart);
                        sequences << seq;
                    }
                    structuredState = Outside; structuredCode.clear();
                    structuredStart = -1; structuredInterruptionStart = -1; structuredReparansStart = -1;
                }
                if ((structuredCode == "DEL") && (structuredState == Interregnum) && !(disAll.split("+").contains(structuredCode + "_"))) {
                    if (createSequences) {
                        Sequence *seq = new Sequence(structuredStart, i - 1, structuredCode);
                        seq->setAttribute("indexInterruptionPoint", i - 1);
                        seq->setAttribute("indexReparans", -1);
                        sequences << seq;
                    }
                    structuredState = Outside; structuredCode.clear();
                    structuredStart = -1; structuredInterruptionStart = -1; structuredReparansStart = -1;
                }
                if ((insideComplexDisfluency == true) && !(disAll.split("+").contains(complexCode))) {
                    if (createSequences) sequences << new Sequence(complexStart, i - 1, complexCode);
                    insideComplexDisfluency = false; complexCode.clear();
                }
                foreach (QString dis, disAll.split("+")) {
                    dis = dis.trimmed();
                    if (dis.isEmpty()) continue;
                    QString code = QString(dis).remove("*").remove("_").section(":", 0, 0);
                    if      (d->codesSkipped.contains(code)) {
                        // OK, skip code
                    }
                    else if (d->codesSingleToken.contains(code)) {
                        if (createSequences) sequences << new Sequence(i, i, code);
                    }
                    else if (d->codesRepetition.contains(code)) {
                        if (repetitionState == Outside) {
                            if      (dis.contains("*"))     { repetitionState = Interregnum; repetitionCode = code; repetitionStart = i; repetitionInterruptionStart = i; }
                            else if (dis.contains("_"))     { hasError = true; errorMessage = "Reparans before Reparandum"; }
                            else if (dis.contains(":edt"))  { hasError = true; errorMessage = "Editing term before Reparandum"; }
                            else                            { repetitionState = Reparandum; repetitionCode = code; repetitionStart = i; }
                        }
                        else if ((repetitionState == Reparandum) && (repetitionCode == code)) {
                            if      (dis.contains("*"))     { repetitionState = Interregnum; repetitionInterruptionStart = i; }
                            else if (dis.contains("_"))     { hasError = true; errorMessage = "Reparans before Interruption Point"; }
                            else if (dis.contains(":edt"))  { hasError = true; errorMessage = "Editing term before Interruption Point"; }
                        }
                        else if ((repetitionState == Interregnum) && (repetitionCode == code)) {
                            if      (dis.contains("*"))     { hasError = true; errorMessage = "Second interruption point before end of sequence"; }
                            else if (dis.contains("_"))     { repetitionState = Reparans;  repetitionReparansStart = i; }
                        }
                        else if ((repetitionState == Reparans) && (repetitionCode == code)) {
                            if (!dis.contains("_"))         { hasError = true; errorMessage = "Superfluous annotation after reparans"; }
                        }
                        else {
                            hasError = true; errorMessage = "Unknown error while processing repetition sequence";
                        }
                        // qDebug() << code << repetitionState;
                    }
                    else if (d->codesStructured.contains(code)) {
                        if (structuredState == Outside) {
                            if      (dis.contains("*"))     { structuredState = Interregnum; structuredCode = code; structuredStart = i; structuredInterruptionStart= i; }
                            else if (dis.contains("_"))     { hasError = true; errorMessage = "Reparans before Reparandum"; }
                            else if (dis.contains(":edt"))  { hasError = true; errorMessage = "Editing term before Reparandum"; }
                            else                            { structuredState = Reparandum; structuredCode = code; structuredStart = i; }
                        }
                        else if ((structuredState == Reparandum) && (structuredCode == code)) {
                            if      (dis.contains("*"))     { structuredState = Interregnum; structuredInterruptionStart = i; }
                            else if (dis.contains("_"))     { hasError = true; errorMessage = "Reparans before Interruption Point"; }
                            else if (dis.contains(":edt"))  { hasError = true; errorMessage = "Editing term before Interruption Point"; }
                        }
                        else if ((structuredState == Interregnum) && (structuredCode == code)) {
                            if      (dis.contains("*"))     { hasError = true; errorMessage = "Second interruption point before end of sequence"; }
                            else if (dis.contains("_"))     { structuredState = Reparans; structuredReparansStart = i; }
                        }
                        else if ((structuredState == Reparans) && (structuredCode == code)) {
                            if (!dis.contains("_"))         { hasError = true; errorMessage = "Superfluous annotation after reparans"; }
                        }
                        else {
                            hasError = true; errorMessage = "Unknown error while processing structured sequence";
                        }
                        // qDebug() << code << structuredState;
                    }
                    else if (d->codesComplex.contains(code)) {
                        if (!insideComplexDisfluency) {
                            insideComplexDisfluency = true; complexCode = code; complexStart = i;
                        }
                    }
                    else {
                        hasError = true; errorMessage = QString("Invalid disfluency code: %1 (code is %2)").arg(disAll).arg(code);
                    }
                    // Manage errors
                    if (hasError) {
                        errors << QString("%1\t%2\t%3\t%4").arg(annotationID).arg(speakerID).arg(i+1).arg(errorMessage);
                        // Reset
                        hasError = false;
                        errorMessage.clear();
                        repetitionState = Outside;       repetitionCode.clear();
                        structuredState = Outside;       structuredCode.clear();
                        insideComplexDisfluency = false; complexCode.clear();
                    }
                } // next dis code
            } // next token
            if (errors.isEmpty()) {
                if (createSequences) {
                    addExtraDataToSequences(sequences, tiers);
                    createDisfluencySequenceAnnotationLevel(com->repository());
                    SequenceTier *tier_seq = new SequenceTier(d->sequencesLevel, sequences, tier_tokens);
                    com->repository()->annotations()->saveTier(annotationID, speakerID, tier_seq);
                    ret.append(QString("%1\t%2\tOK. Created %3 sequences.\n").arg(annotationID).arg(speakerID).arg(sequences.count()));
                    delete tier_seq;
                } else {
                    ret.append(QString("%1\t%2\tOK\n").arg(annotationID).arg(speakerID));
                }
            } else {
                ret.append(errors.join("\n")).append("\n");
            }
        }
        qDeleteAll(tiersAll);
    }
    if (ret.endsWith("\n")) ret.chop(1);
    return ret.trimmed();
}


void SequencerDisfluencies::addExtraDataToSequences(QList<Sequence *> sequences, AnnotationTierGroup *tiers)
{
    IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->annotationLevel);
    if (!tier_tokens) return;
    IntervalTier *tier_response = tiers->getIntervalTierByName("response");

    foreach (Sequence *seq, sequences) {
        int startSequence = seq->indexFrom();
        int endSequence = seq->indexTo();
        int startIP = seq->attribute("indexInterruptionPoint").toInt();
        int startReparans = seq->attribute("indexReparans").toInt();
        QString code = seq->text();
        QString token_text, annotated_text;
        // Sequence text
        for (int i = startSequence; i <= endSequence; ++i) {
            token_text.append(tier_tokens->at(i)->text()).append(" ");
            annotated_text.append(tier_tokens->at(i)->text()).append("/")
                          .append(tier_tokens->at(i)->attribute(d->annotationAttribute).toString()).append(" ");
        }
        if (token_text.length() > 1) token_text.chop(1);
        if (annotated_text.length() > 1) annotated_text.chop(1);
        seq->setAttribute("textSequence", token_text);
        seq->setAttribute("textSequenceAnnotated", annotated_text);
        // Only for repetitions and structured disfluencies
        if (d->codesRepetition.contains(code) || d->codesStructured.contains(code)) {
            // Text reparandum
            int endReparandum = (startIP > 0) ? startIP : startReparans - 1;
            token_text.clear();
            for (int i = startSequence; (i >= 0) && (i <= endReparandum); ++i) {
                token_text.append(tier_tokens->at(i)->text()).append(" ");
            }
            if (token_text.length() > 1) token_text.chop(1);
            seq->setAttribute("textReparandum", token_text);
            // Text interregnum
            if (startIP >= 0) {
                int endInterregnum = startReparans - 1;
                token_text.clear();
                for (int i = startIP + 1; (i >= 0) && (i <= endInterregnum); ++i) {
                    token_text.append(tier_tokens->at(i)->text()).append(" ");
                }
                if (token_text.length() > 1) token_text.chop(1);
                seq->setAttribute("textInterregnum", token_text);
            }
            // Text reparans
            token_text.clear();
            for (int i = startReparans; (i >= 0) && (i <= endSequence); ++i) {
                token_text.append(tier_tokens->at(i)->text()).append(" ");
            }
            if (token_text.length() > 1) token_text.chop(1);
            seq->setAttribute("textReparans", token_text);
        }
        // Response
        if (tier_response) {
            Interval *response = tier_response->intervalAtTime(tier_tokens->at(startSequence)->tCenter());
            if (response) seq->setAttribute("response", response->text());
        }
    }
}
