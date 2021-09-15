#ifndef DISFLUENCYANALYSER_H
#define DISFLUENCYANALYSER_H

#include <QString>
#include <QList>

class DisfluencyAnalyser
{
public:
    DisfluencyAnalyser();

    class Token {
    public:
        Token(QString t, QString d) { text = t; tag = d; i = -1; }
        QString text;
        QString tag;
        int i;
    };

    static QList<QString> annotateRepetition(QList<Token> &input);
    static QList<QString> annotateInsertion(QList<Token> &input);
    static QList<QString> annotateSubstitution(QList<Token> &input);
    static QList<QString> annotateDeletion(QList<Token> &input);
    static QList<QString> annotateComplexBase(QList<Token> &input);

private:
    static QList<QString> prepareInput(QList<Token> &input);
    static QList<QString> mergeResults(const QList<Token> &input, const QList<QString> &tags);
};

#endif // DISFLUENCYANALYSER_H
