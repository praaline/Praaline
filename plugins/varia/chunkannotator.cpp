#include <QObject>
#include <QString>
#include <QList>
#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusAnnotation.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"

#include "chunkannotator.h"

ChunkAnnotator::ChunkAnnotator(IntervalTier *tier_tokens, IntervalTier *tier_chunks, const QString &attributePOS) :
    m_tier_tokens(tier_tokens), m_tier_chunks(tier_chunks), m_attributePOS(attributePOS), m_usingBIO(true)
{
    m_numberOfUnits = m_tier_tokens->count();
}

ChunkAnnotator::~ChunkAnnotator()
{
}

IntervalTier *ChunkAnnotator::getChunkTier()
{
    return m_tier_chunks;
}

void ChunkAnnotator::annotate(const QString &filenameModel)
{
    m_tier_chunks->clear();
    m_tier_chunks->copyIntervalsFrom(m_tier_tokens, false);
    annotateFromCRF(filenameModel);

    // Post-processing
    if (m_usingBIO) {
        if (m_tier_chunks->count() != m_tier_tokens->count())
            return; // error
        for (int i = 0; i < m_tier_chunks->count(); i++) {
            Interval *chunk = m_tier_chunks->interval(i);
            Interval *token = m_tier_tokens->interval(i);
            if (!chunk || !token) break;
            QString pos = token->attribute(m_attributePOS).toString();

            if (pos == "INTROD")
                chunk->setText("INTROD-B"); // hack for c'est, remove after better training!
            if (pos.startsWith("ITJ"))
                chunk->setText("ITJ-B"); // euh
//            if (pos == "PRO:rel")
//                chunk->setText("Srel-B");
//            if (pos == "ADV:adv")
//                chunk->setText("AdP-B");

            Interval *chunk_before = (i > 0) ? m_tier_chunks->interval(i - 1) : 0;
            Interval *chunk_after = (i < m_tier_chunks->count() - 1) ? m_tier_chunks->interval(i + 1) : 0;
            if (chunk && chunk_before && chunk_after) {
                if (token->isPauseSilent()) {
                    if (token->duration().toDouble() >= 0.250)
                        chunk->setText("_");
                    else {
                        QString before = chunk_before->text();
                        QString after = chunk_after->text();

                        if (before.mid(0, before.length() - 2) == after.mid(0, after.length() - 2))
                            chunk->setText(before.mid(0, before.length() - 2).append("-I"));
                        else
                            chunk->setText("_");
                    }
                }
            }
            else {
                if (token->isPauseSilent()) chunk->setText("_");
            }
        }
        // Format results, from BIO format to intervals
        int i = m_tier_chunks->count() - 1;
        while (i > 0) {
            QString A = m_tier_chunks->interval(i - 1)->text();
            QString B = m_tier_chunks->interval(i)->text();
            // Merge when: the tags are the same and the sequence is -B, -I or -I, -I.
            bool merge = (A.mid(0, A.length() - 2) == B.mid(0, B.length() - 2));
            merge = merge && (((A.right(2) == "-B") && (B.right(2) == "-I")) ||
                              ((A.right(2) == "-I") && (B.right(2) == "-I")));
            // bool merge = ((A.left(2) == "B-") && (B.left(2) == "I-") && (A.mid(2) == B.mid(2)));
            if (merge) {
                Interval *merged = m_tier_chunks->merge(i - 1, i);
                merged->setText(A); // keep "B-", it will be removed in the next pass!
            }
            else {
                // m_tier_chunks->interval(i)->setText(B.mid(2));
                m_tier_chunks->interval(i)->setText(B.mid(0, B.length() - 2));
            }
            i--;
        }
    } else {
        if (m_tier_chunks->count() != m_tier_tokens->count())
            return; // error
        for (int i = 0; i < m_tier_chunks->count(); i++) {
            Interval *chunk = m_tier_chunks->interval(i);
            Interval *token = m_tier_tokens->interval(i);
            if (!chunk || !token) break;
            QString pos = token->attribute(m_attributePOS).toString();

            if (pos == "INTROD")
                chunk->setText("INTROD"); // hack for c'est, remove after better training!
            if (pos.startsWith("ITJ"))
                chunk->setText("ITJ"); // euh

            Interval *chunk_before = (i > 0) ? m_tier_chunks->interval(i - 1) : 0;
            Interval *chunk_after = (i < m_tier_chunks->count() - 1) ? m_tier_chunks->interval(i + 1) : 0;
            if (chunk && chunk_before && chunk_after) {
                if (token->isPauseSilent()) {
                    if (token->duration().toDouble() >= 0.250)
                        chunk->setText("_");
                    else {
                        if (chunk_before->text() == chunk_after->text())
                            chunk->setText(chunk_before->text());
                        else
                            chunk->setText("_");
                    }
                }
            }
            else {
                if (token->isPauseSilent()) chunk->setText("_");
            }
        }
        // Merge
        int i = m_tier_chunks->count() - 1;
        while (i > 0) {
            QString A = m_tier_chunks->interval(i - 1)->text();
            QString B = m_tier_chunks->interval(i)->text();
            if (A == B) {
                Interval *merged = m_tier_chunks->merge(i - 1, i);
                merged->setText(A);
            }
            i--;
        }
    }
}

void ChunkAnnotator::writeTraining(QTextStream &out)
{
    writeCRFFile(out, true);
}

bool ChunkAnnotator::isEndOfSequence(int i)
{
    Interval *token = m_tier_tokens->interval(i);
    if (!token) return true;
    if (token->isPauseSilent() && token->duration().toDouble() >= 0.250)
        return true;
    else
        return false;
}

bool ChunkAnnotator::skipUnit(int i)
{
    Q_UNUSED(i)
    return false;
}

void ChunkAnnotator::writeUnitToCRF(QTextStream &out, int i, bool isTraining)
{
    Interval *token = m_tier_tokens->interval(i);

    QString tokenText = (token) ? token->text().replace("\t", "").replace("\n", "").replace(" ", "_") : "";
    if (tokenText.isEmpty()) tokenText = "_";
    out << tokenText << "\t";

    QString posText = (token) ? token->attribute(m_attributePOS).toString().replace("\t", "").replace("\n", "") : "";
    if (posText.isEmpty()) posText = "_";
    // FOR FRENCH
    posText = posText.replace(":pred", "").replace(":aux", "");
    out << posText << "\t";

    if (isTraining) {
        QString chunkText = "_";
        foreach (Interval *chunk, m_tier_chunks->getIntervalsOverlappingWith(token)) {
            chunkText = chunk->text();
        }
        out << chunkText;
    }
    out << "\n";
}

void ChunkAnnotator::decodeUnitFromCRF(const QString &line, int i)
{
    Interval *chunk = m_tier_chunks->interval(i);
    if (!chunk) return;
    QStringList fields = line.split("\t");
    QString result = fields.at(2).split("/").at(0);
    if (result.isEmpty()) return;
    if (result == "O") return;
    chunk->setText(result);
}

