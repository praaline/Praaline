#ifndef VALIBELPROCESSOR_H
#define VALIBELPROCESSOR_H

#include <QString>
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

class ValibelProcessor
{
public:
    ValibelProcessor();
    ~ValibelProcessor();

    static void importValibelFile(Corpus *corpus, const QString &filename);
    static void tokenise(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);
    static QList<QString> splitToken(QString input);
    static void tokmin_punctuation(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);
    static void pauses(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);

private:
    static void mergeInsideParentheses(QString &input);
    static QString formatSegment(QString input);

};

#endif // VALIBELPROCESSOR_H
