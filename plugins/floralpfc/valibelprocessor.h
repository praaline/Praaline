#ifndef VALIBELPROCESSOR_H
#define VALIBELPROCESSOR_H

#include <QString>
#include "pncore/corpus/CorpusCommunication.h"
using namespace Praaline::Core;

class ValibelProcessor
{
public:
    ValibelProcessor();
    ~ValibelProcessor();

    static void importValibelFile(Corpus *corpus, const QString &filename);
    static void tokenise(const QList<CorpusCommunication *> &communications);
    static QList<QString> splitToken(QString input);
    static void tokmin_punctuation(const QList<CorpusCommunication *> &communications);
    static void pauses(const QList<CorpusCommunication *> &communications);

private:
    static void mergeInsideParentheses(QString &input);
    static QString formatSegment(QString input);

};

#endif // VALIBELPROCESSOR_H
