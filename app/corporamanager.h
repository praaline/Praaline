#ifndef CORPORAMANAGER_H
#define CORPORAMANAGER_H

#include <QObject>
#include <QMap>
#include "pncore/corpus/corpus.h"
#include "pngui/observers/corpusobserver.h"

class CorporaManager : public QObject
{
    Q_OBJECT
public:
    explicit CorporaManager(QObject *parent = 0);
    ~CorporaManager();

    QStringList listCorporaIDs();
    QStringList listCorporaNames();

    QPointer<Corpus> activeCorpus();
    QPointer<Corpus> corpusByID(const QString &corpusID);
    QPointer<Corpus> corpusByName(const QString &corpusName);

    void addCorpus(QPointer<Corpus> corpus);
    void removeCorpus(const QString &corpusID);

    void setActiveCorpus(const QString &corpusID);
    QString activeCorpusID() const;

    CorpusObserver *activeCorpusObserver();

signals:
    void corpusAdded(const QString &corpusID);
    void corpusRemoved(const QString &corpusID);
    void activeCorpusChanged(const QString &newActiveCorpusID);

public slots:

private:
    QMap<QString, QPointer<Corpus> > m_corpora;
    QString m_activeCorpusID;
};

#endif // CORPORAMANAGER_H
