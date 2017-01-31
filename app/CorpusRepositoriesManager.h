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
    explicit CorpusRepositoriesManager(QObject *parent = 0);
    ~CorpusRepositoriesManager();

    // Corpus Repositories
    // ==========================================================================================================================
    void addCorpusRepository(Praaline::Core::CorpusRepository *repository);
    void removeCorpusRepository(const QString &repositoryID);

    QStringList listCorpusRepositoryIDs() const;

    QString activeCorpusRepositoryID() const;
    void setActiveCorpusRepository(const QString &repositoryID);

    QPointer<Praaline::Core::CorpusRepository> activeCorpusRepository();
    QPointer<Praaline::Core::CorpusRepository> corpusRepositoryByID(const QString &repositoryID);

    bool isRepositoryDefinitionAlreadyOpen(const QString &filenameRepositoryDefinition);

    // Corpora
    // ==========================================================================================================================
    QStringList listAvailableCorpusIDs(const QString repositoryID = QString());
    QStringList listOpenCorpusIDs(const QString &repositoryID = QString());

    QPointer<Praaline::Core::Corpus> createCorpus(const QString &corpusID, const QString &repositoryID = QString());
    QPointer<Praaline::Core::Corpus> openCorpus(const QString &corpusID, const QString &repositoryID = QString());
    QList<QPointer<Praaline::Core::Corpus> > listOpenCorpora() const;
    bool saveCorpusMetadata(const QString &corpusID = QString());
    bool removeCorpus(const QString &corpusID);

    // Observer
    // ==========================================================================================================================
    CorpusObserver *corpusObserverForRepository(const QString &repositoryID);
    CorpusObserver *activeCorpusObserver();

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
