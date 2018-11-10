#ifndef CONLLUREADER_H
#define CONLLUREADER_H

#include <QString>
#include "UDSentence.h"

class CoNLLUReader
{
public:
    static bool readCoNLLU(const QString &filename, QList<UDSentence> &sentences);

private:
    CoNLLUReader();
};

#endif // CONLLUREADER_H
