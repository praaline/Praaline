#include <QString>
#include <QFile>
#include <QTextStream>
#include "recognitionresult.h"

RecognitionResult::RecognitionResult()
{

}

RecognitionResult::~RecognitionResult()
{

}

bool RecognitionResult::readFile(const QString &filename)
{
    QFile fileResult(filename);
    if ( !fileResult.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    QTextStream result(&fileResult);
    result.setCodec("UTF-8");

    tokens.clear();
    while (!result.atEnd()) {
        QString line = result.readLine();
        if (line.isEmpty()) continue;
        QStringList fields = line.split(" ");
        if (fields.count() != 4) continue;
        QString tok = fields[0];
        if (tok.startsWith("[") || tok.startsWith("<")) continue;
        tok = tok.replace("(1)", "").replace("(2)", "").replace("(3)", "").replace("(4)", "").replace("(5)", "")
                .replace("(6)", "").replace("(7)", "").replace("(8)", "");
        bool convertOK;
        double start = fields[1].toDouble(&convertOK);
        if (!convertOK) continue;
        double end = fields[2].toDouble(&convertOK);
        if (!convertOK) continue;
        double score = fields[3].toDouble(&convertOK);
        if (!convertOK) continue;
        tokens << Token(tok, RealTime::fromSeconds(start), RealTime::fromSeconds(end), score);
    }

    fileResult.close();
    return true;
}

QList<Interval *> RecognitionResult::createIntervalList()
{
    QList<Interval *> intervals;
    foreach (Token token, tokens) {
        intervals << new Interval(token.start, token.end, token.text);
    }
    return intervals;
}
