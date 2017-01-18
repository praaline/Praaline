#include "pncore/datastore/CorpusRepository.h"
using namespace Praaline::Core;

#include "pngui/observers/CorpusObserver.h"

#include "CorpusRepositoriesManager.h"


CorpusRepositoriesManager::CorpusRepositoriesManager(QObject *parent) : QObject(parent)
{
}

CorpusRepositoriesManager::~CorpusRepositoriesManager()
{
}

QStringList CorpusRepositoriesManager::listCorpusRepositoryIDs()
{
    return m_repositories.keys();
}

QPointer<CorpusRepository> CorpusRepositoriesManager::activeCorpusRepository()
{
    return m_repositories.value(m_activeCorpusRepositoryID, Q_NULLPTR);
}

QPointer<CorpusRepository> CorpusRepositoriesManager::corpusRepositoryByID(const QString &repositoryID)
{
    return m_repositories.value(repositoryID, Q_NULLPTR);
}

QString CorpusRepositoriesManager::activeCorpusRepositoryID() const
{
    return m_activeCorpusRepositoryID;
}

void CorpusRepositoriesManager::addCorpusRepository(CorpusRepository *repository)
{
    if (!repository) return;
    m_repositories.insert(repository->ID(), repository);
    emit corpusRepositoryAdded(repository->ID());
}

void CorpusRepositoriesManager::removeCorpusRepository(const QString &repositoryID)
{
    if (m_repositories.contains(repositoryID)) {
        m_repositories.remove(repositoryID);
        emit corpusRepositoryRemoved(repositoryID);
        if (m_activeCorpusRepositoryID == repositoryID) {
            if (m_repositories.isEmpty()) m_activeCorpusRepositoryID = "";
            else m_activeCorpusRepositoryID = m_repositories.keys().first();
            emit activeCorpusRepositoryChanged(m_activeCorpusRepositoryID);
        }
    }
}

void CorpusRepositoriesManager::setActiveCorpusRepository(const QString &repositoryID)
{
    if (m_activeCorpusRepositoryID == repositoryID) return;
    if (!m_repositories.contains(repositoryID)) return;
    m_activeCorpusRepositoryID = repositoryID;
    emit activeCorpusRepositoryChanged(repositoryID);
}

CorpusObserver *CorpusRepositoriesManager::activeCorpusObserver()
{
    QList<QObject *> listCorpusObservers;
    listCorpusObservers = OBJECT_MANAGER->registeredInterfaces("CorpusObserver");
    foreach (QObject* obj, listCorpusObservers) {
        CorpusObserver *obs = qobject_cast<CorpusObserver *>(obj);
        if (obs && obs->repository()) {
            if (obs->repository()->ID() == m_activeCorpusRepositoryID)
                return obs;
        }
    }
    return Q_NULLPTR;
}

