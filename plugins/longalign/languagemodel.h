#ifndef LANGUAGEMODEL_H
#define LANGUAGEMODEL_H

#include <QString>
#include "pncore/annotation/intervaltier.h"

class LanguageModel
{
public:
    LanguageModel();
    ~LanguageModel();

    static bool createSentencesAndVocabularyFromTokens(const QString &filenameBase, QList<Interval *> tokens);
    static bool createModel(const QString &filenameBase, bool createDMP);

};

#endif // LANGUAGEMODEL_H
