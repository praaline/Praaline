#ifndef CORPUSDATASTORE_H
#define CORPUSDATASTORE_H

/*
    Praaline - Core module - Datastores
    Copyright (c) 2011-2017 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include <QObject>
#include <QPointer>
#include <QString>

namespace Praaline {
namespace Core {

class CorpusObject;
class CorpusRepository;

// Internal class
class CorpusDatastore : public QObject
{
    Q_OBJECT
public:
    CorpusDatastore(CorpusRepository *repository, QObject *parent = 0);
    virtual ~CorpusDatastore() {}

    // Back reference to the repository where the corpus object is stored
    // ==========================================================================================================================
    void setRepository(CorpusObject *object);

protected:
    QPointer<CorpusRepository> m_repository;
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSDATASTORE_H
