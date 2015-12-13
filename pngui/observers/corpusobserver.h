#ifndef CORPUSOBSERVER_H
#define CORPUSOBSERVER_H

#include <QObject>
#include <QPointer>
#include "pncore/corpus/corpus.h"
#include "../external/qtilities/include/QtilitiesCore/QtilitiesCore"
#include "../external/qtilities/include/QtilitiesCoreGui/QtilitiesCoreGui"

using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

class CorpusExplorerTreeNodeCommunication : public TreeNode
{
    Q_OBJECT
public:
    CorpusExplorerTreeNodeCommunication(QPointer<CorpusCommunication> com);
    QPointer<CorpusCommunication> communication;
private slots:
    void recordingAdded(QPointer<CorpusRecording> recording);
    void recordingDeleted(QString communicationID, QString recordingID);
    void annotationAdded(QPointer<CorpusAnnotation> annotation);
    void annotationDeleted(QString communicationID, QString annotationID);
};

class CorpusExplorerTreeNodeSpeaker : public TreeNode
{
    Q_OBJECT
public:
    CorpusExplorerTreeNodeSpeaker(QPointer<CorpusSpeaker> spk);
    QPointer<CorpusSpeaker> speaker;
};

class CorpusExplorerTreeNodeRecording : public TreeNode
{
    Q_OBJECT
public:
    CorpusExplorerTreeNodeRecording(QPointer<CorpusRecording> rec);
    QPointer<CorpusRecording> recording;
};

class CorpusExplorerTreeNodeAnnotation : public TreeNode
{
    Q_OBJECT
public:
    CorpusExplorerTreeNodeAnnotation(QPointer<CorpusAnnotation> annot);
    QPointer<CorpusAnnotation> annotation;
};

class CorpusObserver : public QObject
{
    Q_OBJECT
public:
    CorpusObserver(QPointer<Corpus> corpus, QObject *parent = 0);
    ~CorpusObserver();

    QPointer<Corpus> corpus() const { return m_corpus; }
    TreeNode *nodeCommunications() const { return m_nodeCommunications; }
    TreeNode *nodeSpeakers() const { return m_nodeSpeakers; }
    TreeNode *nodeCorpus() const { return m_nodeCorpus; }

    void refresh();

    QStringList communicationsGrouping() const { return m_groupAttributeIDsCommunication; }
    QStringList speakersGrouping() const { return m_groupAttributeIDsSpeaker; }
    void setCommunicationsGrouping(QStringList groupAttributeIDs);
    void setSpeakersGrouping(QStringList groupAttributeIDs);

    CorpusDefinition &definition() { return m_definition; }
    void setDefinition(const CorpusDefinition &definition) { m_definition = definition; }

signals:

private slots:
    void communicationAdded(QPointer<CorpusCommunication> communication);
    void communicationDeleted(QString communicationID);
    void speakerAdded(QPointer<CorpusSpeaker> speaker);
    void speakerDeleted(QString speakerID);

private:
    QPointer<Corpus> m_corpus;
    TreeNode *m_nodeCommunications;
    TreeNode *m_nodeSpeakers;
    TreeNode *m_nodeCorpus;

    QStringList m_groupAttributeIDsCommunication;
    QStringList m_groupAttributeIDsSpeaker;

    CorpusDefinition m_definition;

    void clear();
    void buildTree();
    QString categoryString(CorpusObject *obj, QStringList groupAttributeIDs);
};

#endif // CORPUSOBSERVER_H
