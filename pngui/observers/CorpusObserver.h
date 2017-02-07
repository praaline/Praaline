#ifndef CORPUSOBSERVER_H
#define CORPUSOBSERVER_H

#include <QObject>
#include <QPointer>

#include "pncore/datastore/CorpusRepository.h"
namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class CorpusRecording;
class CorpusAnnotation;
class CorpusSpeaker;
class CorpusObject;
class CorpusObjectInfo;
}
}

#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

struct CorpusObserverData;
class CorpusObserver : public QObject
{
    Q_OBJECT
public:
    CorpusObserver(QPointer<Praaline::Core::CorpusRepository> repository, QObject *parent = 0);
    ~CorpusObserver();

    QPointer<Praaline::Core::CorpusRepository> repository() const;
    TreeNode *nodeRepository() const;

    void addCorpus(QPointer<Praaline::Core::Corpus> corpus);
    void removeCorpus(QString corpusID);

    void setCommunicationsGrouping(QStringList groupAttributeIDs);
    void setSpeakersGrouping(QStringList groupAttributeIDs);

    void refresh();

private:
    CorpusObserverData *d;
};

class CorpusExplorerTreeNodeCorpus : public TreeNode
{
    Q_OBJECT
public:
    CorpusExplorerTreeNodeCorpus(const QString &corpusID);

    QPointer<Praaline::Core::Corpus> corpus() const { return m_corpus; }
    void setCorpus(QPointer<Praaline::Core::Corpus> corpus = 0);

    TreeNode *nodeCommunications() const { return m_nodeCommunications; }
    TreeNode *nodeSpeakers() const { return m_nodeSpeakers; }

    void refresh();

    QStringList communicationsGrouping() const { return m_groupAttributeIDsCommunication; }
    QStringList speakersGrouping() const { return m_groupAttributeIDsSpeaker; }
    void setCommunicationsGrouping(QStringList groupAttributeIDs);
    void setSpeakersGrouping(QStringList groupAttributeIDs);

private slots:
    void communicationAdded(QPointer<Praaline::Core::CorpusCommunication> communication);
    void communicationDeleted(QString communicationID);
    void speakerAdded(QPointer<Praaline::Core::CorpusSpeaker> speaker);
    void speakerDeleted(QString speakerID);

private:
    QPointer<Praaline::Core::Corpus> m_corpus;
    TreeNode *m_nodeCommunications;
    TreeNode *m_nodeSpeakers;

    QStringList m_groupAttributeIDsCommunication;
    QStringList m_groupAttributeIDsSpeaker;

    void clear();
    void buildTree();
    QString categoryString(Praaline::Core::CorpusObject *obj, QStringList groupAttributeIDs);
};

class CorpusExplorerTreeNodeCommunication : public TreeNode
{
    Q_OBJECT
public:
    CorpusExplorerTreeNodeCommunication(QPointer<Praaline::Core::CorpusCommunication> com);
    QPointer<Praaline::Core::CorpusCommunication> communication;
private slots:
    void recordingAdded(QPointer<Praaline::Core::CorpusRecording> recording);
    void recordingDeleted(QString communicationID, QString recordingID);
    void annotationAdded(QPointer<Praaline::Core::CorpusAnnotation> annotation);
    void annotationDeleted(QString communicationID, QString annotationID);
};

class CorpusExplorerTreeNodeSpeaker : public TreeNode
{
    Q_OBJECT
public:
    CorpusExplorerTreeNodeSpeaker(QPointer<Praaline::Core::CorpusSpeaker> spk);
    QPointer<Praaline::Core::CorpusSpeaker> speaker;
};

class CorpusExplorerTreeNodeRecording : public TreeNode
{
    Q_OBJECT
public:
    CorpusExplorerTreeNodeRecording(QPointer<Praaline::Core::CorpusRecording> rec);
    QPointer<Praaline::Core::CorpusRecording> recording;
};

class CorpusExplorerTreeNodeAnnotation : public TreeNode
{
    Q_OBJECT
public:
    CorpusExplorerTreeNodeAnnotation(QPointer<Praaline::Core::CorpusAnnotation> annot);
    QPointer<Praaline::Core::CorpusAnnotation> annotation;
};


#endif // CORPUSOBSERVER_H
