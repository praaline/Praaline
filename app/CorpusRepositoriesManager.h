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

class CorpusRepositoriesManager : public QObject
{
    Q_OBJECT
public:
    explicit CorpusRepositoriesManager(QObject *parent = 0);
    ~CorpusRepositoriesManager();

    QStringList listCorpusRepositoryIDs();
    QPointer<Praaline::Core::CorpusRepository> activeCorpusRepository();
    QPointer<Praaline::Core::CorpusRepository> corpusRepositoryByID(const QString &repositoryID);

    void addCorpusRepository(Praaline::Core::CorpusRepository *repository);
    void removeCorpusRepository(const QString &repositoryID);

    void setActiveCorpusRepository(const QString &repositoryID);
    QString activeCorpusRepositoryID() const;

    CorpusObserver *activeCorpusObserver();

    bool isAlreadyOpen(const QString &filenameDefinition);

signals:
    void corpusRepositoryAdded(const QString &repositoryID);
    void corpusRepositoryRemoved(const QString &repositoryID);
    void activeCorpusRepositoryChanged(const QString &newActiveCorpusRepositoryID);

public slots:

private:
    QMap<QString, QPointer<Praaline::Core::CorpusRepository> > m_repositories;
    QString m_activeCorpusRepositoryID;
};

#endif // CORPUSREPOSITORIESMANAGER_H
