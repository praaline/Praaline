#include <QDebug>
#include <QString>
#include <QList>
#include <QMap>
#include <QPointer>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusObjectInfo.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/CorpusRepositoryDefinition.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
using namespace Praaline::Core;

#include "pngui/observers/CorpusObserver.h"

#include "CorpusRepositoriesManager.h"

struct CorpusRepositoriesManagerData {
    QMap<QString, QPointer<CorpusRepository>> repositories;
    QString activeCorpusRepositoryID;
};


CorpusRepositoriesManager::CorpusRepositoriesManager(QObject *parent) :
    QObject(parent), d(new CorpusRepositoriesManagerData)
{
}

CorpusRepositoriesManager::~CorpusRepositoriesManager()
{
    delete d;
}

// ==========================================================================================================================
// Corpus Repositories
// ==========================================================================================================================

// List all the IDs of CorpusRepositories that have been added to the manager.
QStringList CorpusRepositoriesManager::listCorpusRepositoryIDs() const
{
    return d->repositories.keys();
}

// Find a CorpusRepository in the manager by ID. Returns null if not found.
QPointer<CorpusRepository> CorpusRepositoriesManager::corpusRepositoryByID(const QString &repositoryID) const
{
    return d->repositories.value(repositoryID, nullptr);
}

// Add a CorpusRepository to the manager. Called when opening/connecting to a repository.
void CorpusRepositoriesManager::addCorpusRepository(CorpusRepository *repository)
{
    if (!repository) return;
    d->repositories.insert(repository->ID(), repository);
    emit corpusRepositoryAdded(repository->ID());
}

// Remove a CorpusRepository from the manager. Called when closing/disconnecting from the repository.
void CorpusRepositoriesManager::removeCorpusRepository(const QString &repositoryID)
{
    if (d->repositories.contains(repositoryID)) {
        d->repositories.remove(repositoryID);
        emit corpusRepositoryRemoved(repositoryID);
        // Select the next repository and set it active.
        if (d->activeCorpusRepositoryID == repositoryID) {
            if (d->repositories.isEmpty())
                d->activeCorpusRepositoryID = "";
            else
                d->activeCorpusRepositoryID = d->repositories.firstKey();
            emit activeCorpusRepositoryChanged(d->activeCorpusRepositoryID);
        }
    }
}

// Return the active CorpusRepository.
QPointer<CorpusRepository> CorpusRepositoriesManager::activeCorpusRepository() const
{
    return d->repositories.value(d->activeCorpusRepositoryID, nullptr);
}

// Return the active CorpusRepository's ID.
QString CorpusRepositoriesManager::activeCorpusRepositoryID() const
{
    return d->activeCorpusRepositoryID;
}

// Change the active CorpusRepository
void CorpusRepositoriesManager::setActiveCorpusRepository(const QString &repositoryID)
{
    if (d->activeCorpusRepositoryID == repositoryID) return;
    if (!d->repositories.contains(repositoryID)) return;
    d->activeCorpusRepositoryID = repositoryID;
    emit activeCorpusRepositoryChanged(repositoryID);
}

bool CorpusRepositoriesManager::isRepositoryDefinitionAlreadyOpen(const QString &filenameDefinition)
{
    foreach (QPointer<CorpusRepository> repository, d->repositories) {
        if (repository->definition().filenameDefinition == filenameDefinition)
            return true;
    }
    return false;
}

// ==========================================================================================================================
// Observers
// ==========================================================================================================================
CorpusObserver *CorpusRepositoriesManager::corpusObserverForRepository(const QString &repositoryID)
{
    QList<QObject *> listCorpusObservers;
    listCorpusObservers = OBJECT_MANAGER->registeredInterfaces("CorpusObserver");
    foreach (QObject* obj, listCorpusObservers) {
        CorpusObserver *obs = qobject_cast<CorpusObserver *>(obj);
        if (obs && obs->repository()) {
            if (obs->repository()->ID() == repositoryID) return obs;
        }
    }
    return nullptr;
}

CorpusObserver *CorpusRepositoriesManager::activeCorpusObserver()
{
    return corpusObserverForRepository(d->activeCorpusRepositoryID);
}

