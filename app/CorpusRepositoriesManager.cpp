#include <QDebug>
#include <QString>
#include <QList>
#include <QPointer>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusObjectInfo.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/CorpusRepositoryDefinition.h"
#include "pncore/datastore/MetadataDatastore.h"
using namespace Praaline::Core;

#include "pngui/observers/CorpusObserver.h"

#include "CorpusRepositoriesManager.h"

struct CorpusRepositoriesManagerData {
    QMap<QString, QPointer<CorpusRepository> > repositories;
    QString activeCorpusRepositoryID;
    QMap<QString, QPointer<Corpus> > corpora;
};



CorpusRepositoriesManager::CorpusRepositoriesManager(QObject *parent) :
    QObject(parent), d(new CorpusRepositoriesManagerData)
{
}

CorpusRepositoriesManager::~CorpusRepositoriesManager()
{
    delete d;
}

// Corpus Repositories
// ==========================================================================================================================

QStringList CorpusRepositoriesManager::listCorpusRepositoryIDs() const
{
    return d->repositories.keys();
}

QPointer<CorpusRepository> CorpusRepositoriesManager::activeCorpusRepository()
{
    return d->repositories.value(d->activeCorpusRepositoryID, Q_NULLPTR);
}

QPointer<CorpusRepository> CorpusRepositoriesManager::corpusRepositoryByID(const QString &repositoryID)
{
    return d->repositories.value(repositoryID, Q_NULLPTR);
}

QString CorpusRepositoriesManager::activeCorpusRepositoryID() const
{
    return d->activeCorpusRepositoryID;
}

void CorpusRepositoriesManager::addCorpusRepository(CorpusRepository *repository)
{
    if (!repository) return;
    d->repositories.insert(repository->ID(), repository);
    emit corpusRepositoryAdded(repository->ID());
}

void CorpusRepositoriesManager::removeCorpusRepository(const QString &repositoryID)
{
    if (d->repositories.contains(repositoryID)) {
        d->repositories.remove(repositoryID);
        // Delete and remove open corpora
        foreach (QPointer<Corpus> corpus, d->corpora.values()) {
            if (!corpus) continue;
            if (!corpus->repository()) continue;
            if (corpus->repository()->ID() == repositoryID) {
                d->corpora.remove(corpus->ID());
                delete corpus;
            }
        }
        emit corpusRepositoryRemoved(repositoryID);
        // Select next repository
        if (d->activeCorpusRepositoryID == repositoryID) {
            if (d->repositories.isEmpty()) d->activeCorpusRepositoryID = "";
            else d->activeCorpusRepositoryID = d->repositories.keys().first();
            emit activeCorpusRepositoryChanged(d->activeCorpusRepositoryID);
        }
    }
}

void CorpusRepositoriesManager::setActiveCorpusRepository(const QString &repositoryID)
{
    if (d->activeCorpusRepositoryID == repositoryID) return;
    if (!d->repositories.contains(repositoryID)) return;
    d->activeCorpusRepositoryID = repositoryID;
    emit activeCorpusRepositoryChanged(repositoryID);
}

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
    return Q_NULLPTR;
}

CorpusObserver *CorpusRepositoriesManager::activeCorpusObserver()
{
    return corpusObserverForRepository(d->activeCorpusRepositoryID);
}

bool CorpusRepositoriesManager::isRepositoryDefinitionAlreadyOpen(const QString &filenameDefinition)
{
    foreach (QPointer<CorpusRepository> repository, d->repositories.values()) {
        if (repository->definition().filenameDefinition == filenameDefinition)
            return true;
    }
    return false;
}

QStringList CorpusRepositoriesManager::listAvailableCorpusIDs(const QString repositoryID)
{
    QStringList list;
    foreach (QPointer<CorpusRepository> repository, d->repositories.values()) {
        if (!repository) continue;
        if ((!repositoryID.isEmpty()) && (repository->ID() != repositoryID)) continue;
        foreach (CorpusObjectInfo item, repository->listCorporaInfo()) {
            list << item.attribute("corpusID").toString();
        }
    }
    return list;
}

QStringList CorpusRepositoriesManager::listOpenCorpusIDs(const QString &repositoryID)
{
    QStringList list;
    foreach (QPointer<Corpus> corpus, d->corpora) {
        if (!corpus) continue;
        if (!corpus->repository()) continue;
        if (corpus->repository()->ID() == repositoryID || repositoryID.isEmpty())
            list << corpus->ID();
    }
    return list;
}

QPointer<Corpus> CorpusRepositoriesManager::createCorpus(const QString &corpusID, const QString &repositoryID)
{
    QString repID = (repositoryID.isEmpty()) ? activeCorpusRepositoryID() : repositoryID;
    QPointer<CorpusRepository> repository = corpusRepositoryByID(repID);
    if (!repository) return Q_NULLPTR;
    QPointer<Corpus> corpus = new Corpus(corpusID, repository);
    CorpusObserver *obs = corpusObserverForRepository(repID);
    if (obs) obs->addCorpus(corpus);
    d->corpora.insert(corpusID, corpus);
    qDebug() << "Created corpus" << corpus;
    return corpus;
}

QPointer<Corpus> CorpusRepositoriesManager::openCorpus(const QString &corpusID, const QString &repositoryID)
{
    QString repID = (repositoryID.isEmpty()) ? activeCorpusRepositoryID() : repositoryID;
    QPointer<CorpusRepository> repository = corpusRepositoryByID(repID);
    if (!repository) return Q_NULLPTR;
    if (!repository->metadata()) return Q_NULLPTR;
    QPointer<Corpus> corpus = repository->metadata()->getCorpus(corpusID);
    CorpusObserver *obs = corpusObserverForRepository(repID);
    if (obs) {
        obs->addCorpus(corpus);
    }
    d->corpora.insert(corpusID, corpus);
    return corpus;
}

QPointer<Corpus> CorpusRepositoriesManager::getCorpusAlreadyOpen(const QString &corpusID)
{
    return d->corpora.value(corpusID, Q_NULLPTR);
}

QList<QPointer<Corpus> > CorpusRepositoriesManager::listOpenCorpora() const
{
    return d->corpora.values();
}

bool CorpusRepositoriesManager::saveCorpusMetadata(const QString &corpusID)
{
    bool result(true);
    foreach (QString id, d->corpora.keys()) {
        if ((!corpusID.isEmpty()) && (corpusID != id)) continue;
        QPointer<Corpus> corpus = d->corpora.value(id);
        if (corpus) result = result && corpus->save();
    }
    return result;
}

bool CorpusRepositoriesManager::removeCorpus(const QString &corpusID)
{
    QPointer<Corpus> corpus = d->corpora.value(corpusID, Q_NULLPTR);
    if (!corpus) return false;
    QString repositoryID = (corpus->repository()) ? corpus->repository()->ID() : "";
    d->corpora.remove(corpusID);
    delete corpus;
    CorpusObserver *obs = corpusObserverForRepository(repositoryID);
    if (obs) obs->removeCorpus(corpusID);
    return true;
}
