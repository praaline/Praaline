#include <QString>
#include <QList>
#include <QPair>
#include "DisfluencyAnalyser.h"

DisfluencyAnalyser::DisfluencyAnalyser()
{
}

QList<QString> DisfluencyAnalyser::prepareInput(QList<Token> &input)
{
    QList<QString> ret;
    int i = 0;
    for (int j = 0; j < input.count(); j++) {
             if (input[j].tag.contains("FIL")) input[j].tag = "FIL";
        else if (input[j].tag.contains("FST")) input[j].tag = "FST";
        else if (input[j].tag.contains("LEN")) input[j].tag = "LEN";
        else if (input[j].tag.contains("WDP")) input[j].tag = "WDP";
        else if (input[j].tag.contains("SIL")) input[j].tag = "SIL";
        else input[j].tag = "";

        if (input[j].tag.isEmpty() || input[j].tag == "LEN") {
            ret << input[j].text;
            input[j].i = i;
            i++;
        }
        else {
            input[j].i = -1;
        }
    }
    return ret;
}

QList<QString> DisfluencyAnalyser::mergeResults(const QList<Token> &input, const QList<QString> &tags)
{
    QList<QString> ret;
    QString baseCode = tags.first().left(3);
    foreach (Token tok, input) {
        if (tok.i >= 0) {
            QString compositeTag = tags.at(tok.i);
            if (!tok.tag.isEmpty()) compositeTag.append("+").append(tok.tag);
            ret << compositeTag;
        }
        else {
            ret << QString(baseCode).append("+").append(tok.tag);
        }
    }
    return ret;
}

// 0 1 2 3 4 5
// A A A A
// A B A B A B
// A B C A B C
QList<QString> DisfluencyAnalyser::annotateRepetition(QList<Token> &input)
{
    QList<QString> tokens = prepareInput(input);
    int n = tokens.count();
    // create empty tag list
    QList<QString> tags;
    for (int i = 0; i < n; i++) tags.append("");
    // checks
    if (n < 2) return mergeResults(input, tags);
    // search
    int span = 1;
    bool OK = false;
    while (span < n - 1 && OK == false) {
        OK = true;
        for (int i = 0; i < n; i += span) {
            for (int j = 0; j < span; j++) {
                if (i + j >= n)
                    break;
                if (tokens.at(i + j) != tokens.at(j))
                    OK = false;
            }
        }
        if (OK == false) span++;
    }
    if (span < n - 1) {
        int interruptionPoint = n - span - 1;
        for (int i = 0; i < n; i++) {
            if (i < interruptionPoint)
                tags[i] = QString("REP:%1").arg((i % span) + 1);
            else if (i == interruptionPoint)
                tags[i] = QString("REP*:%1").arg((i % span) + 1);
            else
                tags[i] = "REP_";
        }
    }
    else if (n == 2) {
        if (tokens[0] == tokens[1]) {
            tags[0] = "REP*";
            tags[1] = "REP_";
        }
    }
    else {
        // otherwise it was not possible to detect the repetition pattern
        for (int i = 0; i < n; i++)
            tags[i] = "REP";
    }
    return mergeResults(input, tags);
}

// and the second and the third:
//   and the second
//   and the third
// and the second sorry and the third:
//   and the second sorry
//   and the third
int bestInterruptionPointForEditingDisfluency(const QList<QString> &tokens)
{
    // no checks: the calling algorithm should already have performed the necessary checks
    int n = tokens.count();
    // calculate similarity scores for different interruption points
    QList<QPair<int, int> > scores;
    // for each possible interruption point (i)
    for (int i = 0; i < n; i++) {
        // decompose
        QList<QString> A = tokens.mid(0, i+1);
        QList<QString> B = tokens.mid(i+1, -1);
        // count the number of aligned tokens
        int j = 0;
        while (j < qMin(A.count(), B.count())) {
            if (A.at(j) == B.at(j))
                j++;
            else
                break;
        }
        // the score for interruption point at i is j
        scores.append(QPair<int, int>(i, j));
    }
    // Find the maximum score
    int interruptionPoint = scores.first().first; int max = scores.first().second;
    QPair<int, int> score;
    foreach (score, scores) {
        if (score.second > max) {
            interruptionPoint = score.first;
            max = score.second;
        }
    }
    return interruptionPoint;
}

QList<QString> DisfluencyAnalyser::annotateSubstitution(QList<Token> &input)
{
    QList<QString> tokens = prepareInput(input);
    int n = tokens.count();
    // create empty tag list
    QList<QString> tags;
    for (int i = 0; i < n; i++) tags.append("SUB");
    // checks
    if (n < 2) return mergeResults(input, tags);
    // calculate interruption point
    int interruptionPoint = bestInterruptionPointForEditingDisfluency(tokens);
    for (int i = 0; i < n; i++) {
        if (i < interruptionPoint)
            tags[i] = "SUB";
        else if (i == interruptionPoint)
            tags[i] = "SUB*";
        else
            tags[i] = "SUB_";
    }
    return mergeResults(input, tags);
}

QList<QString> DisfluencyAnalyser::annotateInsertion(QList<Token> &input)
{
    QList<QString> tokens = prepareInput(input);
    int n = tokens.count();
    // create empty tag list
    QList<QString> tags;
    for (int i = 0; i < n; i++) tags.append("INS");
    // checks
    if (n < 2) return mergeResults(input, tags);
    // calculate interruption point
    int interruptionPoint = bestInterruptionPointForEditingDisfluency(tokens);
    for (int i = 0; i < n; i++) {
        if (i < interruptionPoint)
            tags[i] = "INS";
        else if (i == interruptionPoint)
            tags[i] = "INS*";
        else
            tags[i] = "INS_";
    }
    return mergeResults(input, tags);
}

QList<QString> DisfluencyAnalyser::annotateDeletion(QList<Token> &input)
{
    QList<QString> tokens = prepareInput(input);
    int n = tokens.count();
    // create basic tag list
    QList<QString> tags;
    for (int i = 0; i < n; i++) {
        tags << "DEL";
        if (i == n-1) tags[i] = "DEL*";
    }
    return mergeResults(input, tags);
}

QList<QString> DisfluencyAnalyser::annotateComplexBase(QList<Token> &input)
{
    QList<QString> tokens = prepareInput(input);
    int n = tokens.count();
    // create basic tag list
    QList<QString> tags;
    for (int i = 0; i < n; i++) {
        tags << "COM";
    }
    return mergeResults(input, tags);
}
