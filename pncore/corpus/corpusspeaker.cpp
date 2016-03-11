#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "corpus.h"
#include "corpusobject.h"
#include "corpusspeaker.h"

CorpusSpeaker::CorpusSpeaker(QObject *parent) :
    CorpusObject(parent)
{
}

CorpusSpeaker::CorpusSpeaker(const QString &ID, QObject *parent) :
    CorpusObject(ID, parent)
{
}

CorpusSpeaker::CorpusSpeaker(CorpusSpeaker *other, QObject *parent) :
    CorpusObject(parent)
{
    if (!other) return;
    m_ID = other->m_ID;
    m_originalID = other->m_originalID;
    m_name = other->m_name;
    m_corpusID = other->m_corpusID;
    copyPropertiesFrom(other);
    setDirty(true);
    setNew(true);
}

QPointer<Corpus> CorpusSpeaker::corpus() const
{
    return qobject_cast<Corpus *>(this->parent());
}

QString CorpusSpeaker::basePath() const
{
    Corpus *myCorpus = qobject_cast<Corpus *>(this->parent());
    if (myCorpus)
        return myCorpus->basePath();
    else
        return QString();
}

QString CorpusSpeaker::baseMediaPath() const
{
    Corpus *myCorpus = qobject_cast<Corpus *>(this->parent());
    if (myCorpus)
        return myCorpus->baseMediaPath();
    else
        return QString();
}

void CorpusSpeaker::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        setDirty(true);
    }
}
