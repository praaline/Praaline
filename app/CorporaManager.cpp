#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "pngui/observers/CorpusObserver.h"

#include "CorporaManager.h"


CorporaManager::CorporaManager(QObject *parent) : QObject(parent)
{

}

CorporaManager::~CorporaManager()
{

}

QStringList CorporaManager::listCorporaIDs()
{
    return m_corpora.keys();
}

QStringList CorporaManager::listCorporaNames()
{
    QStringList list;
    foreach (QPointer<Corpus> corpus, m_corpora) {
        if (!corpus) continue;
        list << corpus->name();
    }
    return list;
}

QPointer<Corpus> CorporaManager::activeCorpus()
{
    return m_corpora.value(m_activeCorpusID, 0);
}

QPointer<Corpus> CorporaManager::corpusByID(const QString &corpusID)
{
    return m_corpora.value(corpusID, 0);
}

QPointer<Corpus> CorporaManager::corpusByName(const QString &corpusName)
{
    foreach (QPointer<Corpus> corpus, m_corpora) {
        if (!corpus) continue;
        if (corpus->name() == corpusName) return corpus;
    }
    return 0;
}

QString CorporaManager::activeCorpusID() const
{
    return m_activeCorpusID;
}

void CorporaManager::addCorpus(QPointer<Corpus> corpus)
{
    if (!corpus) return;
    m_corpora.insert(corpus->ID(), corpus);
    emit corpusAdded(corpus->ID());
}

void CorporaManager::removeCorpus(const QString &corpusID)
{
    if (m_corpora.contains(corpusID)) {
        m_corpora.remove(corpusID);
        if (m_activeCorpusID == corpusID) {
            if (m_corpora.isEmpty()) m_activeCorpusID = "";
            else m_activeCorpusID = m_corpora.keys().first();
            emit activeCorpusChanged(m_activeCorpusID);
        }
        emit corpusRemoved(corpusID);
    }
}

void CorporaManager::setActiveCorpus(const QString &corpusID)
{
    if (m_activeCorpusID == corpusID) return;
    if (!m_corpora.contains(corpusID)) return;
    m_activeCorpusID = corpusID;
    emit activeCorpusChanged(corpusID);
}

CorpusObserver *CorporaManager::activeCorpusObserver()
{
    QList<QObject *> listCorpora;
    listCorpora = OBJECT_MANAGER->registeredInterfaces("CorpusObserver");
    foreach (QObject* obj, listCorpora) {
        CorpusObserver *obs = qobject_cast<CorpusObserver *>(obj);
        if (obs && obs->corpus()) {
            if (obs->corpus()->ID() == m_activeCorpusID)
                return obs;
        }
    }
    return 0;
}

