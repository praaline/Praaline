#ifndef DICTIONARY_FST_H
#define DICTIONARY_FST_H

/*
    DisMo Annotator
    Copyright (c) 2012-2014 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include <QString>
#include <QStringList>
#include <QHash>

namespace DisMoAnnotator {

class DictionaryFST
{
public:
    DictionaryFST(const QString &filename);
    void lookup(QHash<QString, QStringList> &table);
private:
    QString m_filenameFST;
};

}

#endif // DICTIONARY_FST_H
