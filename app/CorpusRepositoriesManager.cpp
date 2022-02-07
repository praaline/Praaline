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
    struct CacheEntry {
        CacheEntry() : repository(nullptr) {}
        explicit CacheEntry(CorpusRepository *repository) : repository(repository) {}

        QPointer<CorpusRepository> repository;
        QList<QPointer<Corpus>> corpora;
    };

    QMap<QString, CacheEntry> cache;
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
    return d->cache.keys();
}

// Find a CorpusRepository in the manager by ID. Returns null if not found.
QPointer<CorpusRepository> CorpusRepositoriesManager::corpusRepositoryByID(const QString &repositoryID) const
{
    if (d->cache.contains(repositoryID))
        return d->cache[repositoryID].repository;
    return nullptr;
}

// Add a CorpusRepository to the manager. Called when opening/connecting to a repository.
void CorpusRepositoriesManager::addCorpusRepository(CorpusRepository *repository)
{
    if (!repository) return;
    d->cache.insert(repository->ID(), CorpusRepositoriesManagerData::CacheEntry(repository));
    emit corpusRepositoryAdded(repository->ID());
}

// Remove a CorpusRepository from the manager. Called when closing/disconnecting from the repository.
void CorpusRepositoriesManager::removeCorpusRepository(const QString &repositoryID)
{
    if (d->cache.contains(repositoryID)) {
        // Delete any corpora cached for this repository. Then remove the entry from the cache emmiting a signal.
        qDeleteAll(d->cache[repositoryID].corpora);
        d->cache.remove(repositoryID);
        emit corpusRepositoryRemoved(repositoryID);
        // Select the next repository and set it active.
        if (d->activeCorpusRepositoryID == repositoryID) {
            if (d->cache.isEmpty())
                d->activeCorpusRepositoryID.clear();
            else
                d->activeCorpusRepositoryID = d->cache.firstKey();
            emit activeCorpusRepositoryChanged(d->activeCorpusRepositoryID);
        }
    }
}

// Return the active CorpusRepository.
QPointer<CorpusRepository> CorpusRepositoriesManager::activeCorpusRepository() const
{
    if (d->cache.contains(d->activeCorpusRepositoryID))
        return d->cache[d->activeCorpusRepositoryID].repository;
    return nullptr;
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
    if (!d->cache.contains(repositoryID)) return;
    d->activeCorpusRepositoryID = repositoryID;
    emit activeCorpusRepositoryChanged(repositoryID);
}

bool CorpusRepositoriesManager::isRepositoryDefinitionAlreadyOpen(const QString &filenameDefinition)
{
    for (auto entry: d->cache) {
        if (!entry.repository) continue;
        if (entry.repository->definition().filenameDefinition == filenameDefinition)
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

// ==========================================================================================================================
// Metadata Cache
// ==========================================================================================================================

QStringList CorpusRepositoriesManager::listAvailableCorpusIDs(const QString &repositoryID) const
{
    QStringList list;
    for (auto entry : d->cache) {
        if (!entry.repository) continue;
        if ((!repositoryID.isEmpty()) && (entry.repository->ID() != repositoryID)) continue;
        foreach (CorpusObjectInfo item, entry.repository->listCorporaInfo()) {
            list << item.attribute("corpusID").toString();
        }
    }
    return list;
}

QPointer<Corpus> CorpusRepositoriesManager::createCorpus(const QString &repositoryID, const QString &corpusID)
{
    // If no repository ID is given, assume the active repository
    QString repID = (repositoryID.isEmpty()) ? activeCorpusRepositoryID() : repositoryID;
    // Ensure that the repository is available (loaded in the cache)
    if (!d->cache.contains(repID)) return nullptr;
    if (!d->cache[repID].repository) return nullptr;
    // Create a new corpus object, owned by the currect CorpusRepositoriesManager (this object).
    QPointer<Corpus> corpus = new Corpus(corpusID, d->cache[repID].repository);
    // Add Corpus object to the cache
    d->cache[repID].corpora << corpus;
    // Update the observer corresponding to the repository, to include the new corpus
    CorpusObserver *obs = corpusObserverForRepository(repID);
    if (obs) obs->addCorpus(corpus);
    // Return the new Corpus object
    qDebug() << "Created corpus: " << corpus;
    return corpus;
}

QPointer<Corpus> CorpusRepositoriesManager::getCorpus(const QString &repositoryID, const QString &corpusID)
{
    // If no repository ID is given, assume the active repository
    QString repID = (repositoryID.isEmpty()) ? activeCorpusRepositoryID() : repositoryID;
    // Ensure that the repository is available (loaded in the cache)
    if (!d->cache.contains(repID)) return nullptr;
    if (!d->cache[repID].repository) return nullptr;
    // Check if the Corpus is already open
    for (QPointer<Corpus> corpus : d->cache[repID].corpora) {
        if (!corpus) continue;
        if (corpus->ID() == corpusID) {
            qDebug() << "Opened corpus from cache: " << corpus;
            return corpus; // found it
        }
    }
    // Otherwise, load Corpus object from the metadata datastore, save it into the cache and return it
    if (!d->cache[repID].repository->metadata()) return nullptr;
    QPointer<Corpus> corpus = d->cache[repID].repository->metadata()->getCorpus(corpusID);
    // Add Corpus object to the cache
    d->cache[repID].corpora << corpus;
    // Update the observer corresponding to the repository, to include the opened corpus
    CorpusObserver *obs = corpusObserverForRepository(repID);
    if (obs) obs->addCorpus(corpus);
    // Return the opened Corpus object
    qDebug() << "Opened corpus from database: " << corpus;
    return corpus;
}

void CorpusRepositoriesManager::removeCorpus(const QString repositoryID, const QString &corpusID)
{
    // If no repository ID is given, assume the active repository
    QString repID = (repositoryID.isEmpty()) ? activeCorpusRepositoryID() : repositoryID;
    // Ensure that the repository is available (loaded in the cache)
    if (!d->cache.contains(repID)) return;
    if (!d->cache[repID].repository) return;
    // Check if the Corpus is open, and if so remove it
    for (int i = d->cache[repID].corpora.count() - 1; i >= 0; i--) {
        QPointer<Corpus> corpus = d->cache[repID].corpora.at(i);
        if (!corpus) continue;
        if (corpus->ID() == corpusID) {
            d->cache[repID].corpora.removeAt(i);
            delete corpus;
            break;
        }
    }
    // Update the observer corresponding to the repository, removing the corpus
    CorpusObserver *obs = corpusObserverForRepository(repID);
    if (obs) obs->removeCorpus(corpusID);
}
