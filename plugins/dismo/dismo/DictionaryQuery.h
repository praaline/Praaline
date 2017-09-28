#ifndef DICTIONARYQUERY_H
#define DICTIONARYQUERY_H

#include <QString>
#include "DictionaryEntry.h"

namespace DisMoAnnotator {

class DictionaryQuery
{
public:
    DictionaryQuery(int i, int j, QString search) : i(i), j(j), search(search) {}
    int i;
    int j;
    QString search;
    QStringList response;

    DictionaryEntry getResponse();
};

}

#endif // DICTIONARYQUERY_H
