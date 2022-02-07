#ifndef CORPUSREPOSITORIESMANAGER_H
#define CORPUSREPOSITORIESMANAGER_H

#include <QObject>
#include <QMap>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusRepository;
}
}

#include "pngui/observers/CorpusObserver.h"

struct CorpusRepositoriesManagerData;

class CorpusRepositoriesManager : public QObject
{
    Q_OBJECT
public:
    explicit CorpusRepositoriesManager(QObject *parent = nullptr);
    ~CorpusRepositoriesManager();

    // Corpus Repositories
    // ==========================================================================================================================
    QStringList listCorpusRepositoryIDs() const;
    QPointer<Praaline::Core::CorpusRepository> corpusRepositoryByID(const QString &repositoryID) const;
    void addCorpusRepository(Praaline::Core::CorpusRepository *repository);
    void removeCorpusRepository(const QString &repositoryID);

    QPointer<Praaline::Core::CorpusRepository> activeCorpusRepository() const;
    QString activeCorpusRepositoryID() const;
    void setActiveCorpusRepository(const QString &repositoryID);

    bool isRepositoryDefinitionAlreadyOpen(const QString &filenameRepositoryDefinition);

    // Observers
    // ==========================================================================================================================
    CorpusObserver *corpusObserverForRepository(const QString &repositoryID);
    CorpusObserver *activeCorpusObserver();

    // Metadata Cache
    // ==========================================================================================================================
    QStringList listAvailableCorpusIDs(const QString &repositoryID) const;
    QPointer<Praaline::Core::Corpus> createCorpus(const QString &repositoryID, const QString &corpusID);
    QPointer<Praaline::Core::Corpus> getCorpus(const QString &repositoryID, const QString &corpusID);
    void removeCorpus(const QString repositoryID, const QString &corpusID);

signals:
    void corpusRepositoryAdded(const QString &repositoryID);
    void corpusRepositoryRemoved(const QString &repositoryID);
    void activeCorpusRepositoryChanged(const QString &newActiveCorpusRepositoryID);
    void corpusOpened(const QString &corpusID, const QString &repositoryID);
    void corpusClosed(const QString &corpusID, const QString &repositoryID);

public slots:

private:
    CorpusRepositoriesManagerData *d;
};

#endif // CORPUSREPOSITORIESMANAGER_H
