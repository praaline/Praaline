#ifndef RECOGNITIONRESULT_H
#define RECOGNITIONRESULT_H

#include <QString>
#include "base/RealTime.h"
#include "pncore/annotation/intervaltier.h"

class RecognitionResult
{
public:
    RecognitionResult();
    ~RecognitionResult();

    class Token {
    public:
        Token (const QString &text, RealTime start, RealTime end, double score) :
            text(text), start(start), end(end), score(score) {}

        QString text;
        RealTime start;
        RealTime end;
        double score;
    };

    QList<Token> tokens;

    bool readFile(const QString &filename);

    QList<Interval *> createIntervalList();
};

#endif // RECOGNITIONRESULT_H
