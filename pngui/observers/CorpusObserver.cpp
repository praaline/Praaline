#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusRecording.h"
#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/corpus/CorpusSpeaker.h"
#include "pncore/corpus/CorpusObjectInfo.h"
using namespace Praaline::Core;

#include "CorpusObserver.h"

// ==============================================================================================================================
// Corpus observer: lists all corpora in a repository
// ==============================================================================================================================
CorpusObserver::CorpusObserver(QPointer<Praaline::Core::CorpusRepository> repository, QObject *parent) :
    QObject(parent), m_repository(repository)
{
    m_nodeRepository = new TreeNode(repository->ID(), this);
//    QList<CorpusObjectInfo> list = m_repository->metadata()->getCorpusObjectInfoList(
//                CorpusObject::Type_Corpus, MetadataDatastore::Selection("", "", ""));
//    foreach (CorpusObjectInfo item, list) {
//        Corpus *corpus = new Corpus(item.ID(), item.name(), item.description(), this);
//        m_repository->metadata()->loadCorpus(corpus);
//        CorpusExplorerTreeNodeCorpus *node = new CorpusExplorerTreeNodeCorpus(corpus);
//        node->refresh();
//        m_nodeRepository->addNode(node);
//    }
    Corpus *corpus = m_repository->metadata()->getCorpus(repository->ID());
    if (corpus) {
        CorpusExplorerTreeNodeCorpus *node = new CorpusExplorerTreeNodeCorpus(corpus);
        node->refresh();
        m_nodeRepository->addNode(node);
    }
}

CorpusObserver::~CorpusObserver()
{
}

void CorpusObserver::setCommunicationsGrouping(QStringList groupAttributeIDs)
{

}

void CorpusObserver::setSpeakersGrouping(QStringList groupAttributeIDs)
{
}

// ==============================================================================================================================
// Corpus tree node
// ==============================================================================================================================

CorpusExplorerTreeNodeCorpus::CorpusExplorerTreeNodeCorpus(QPointer<Corpus> corpus) :
    TreeNode(corpus->ID()), m_corpus(corpus)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus.png"));
    m_nodeCommunications = 0;
    m_nodeSpeakers = 0;
    buildTree();
    connect(corpus, SIGNAL(communicationAdded(QPointer<Praaline::Core::CorpusCommunication>)),
            this, SLOT(communicationAdded(QPointer<Praaline::Core::CorpusCommunication>)));
    connect(corpus, SIGNAL(communicationDeleted(QString)),
            this, SLOT(communicationDeleted(QString)));
    connect(corpus, SIGNAL(speakerAdded(QPointer<Praaline::Core::CorpusSpeaker>)),
            this, SLOT(speakerAdded(QPointer<Praaline::Core::CorpusSpeaker>)));
    connect(corpus, SIGNAL(speakerDeleted(QString)),
            this, SLOT(speakerDeleted(QString)));
}


void CorpusExplorerTreeNodeCorpus::communicationAdded(QPointer<CorpusCommunication> communication)
{
    if (!communication) return;
    CorpusExplorerTreeNodeCommunication *nodeCom = new CorpusExplorerTreeNodeCommunication(communication);
    foreach (CorpusRecording *rec, communication->recordings()) {
        CorpusExplorerTreeNodeRecording *nodeRec = new CorpusExplorerTreeNodeRecording(rec);
        nodeCom->addNode(nodeRec);
    }
    foreach (CorpusAnnotation *annot, communication->annotations()) {
        CorpusExplorerTreeNodeAnnotation *nodeAnnot = new CorpusExplorerTreeNodeAnnotation(annot);
        nodeCom->addNode(nodeAnnot);
    }
    m_nodeCommunications->addNode(nodeCom, QtilitiesCategory(categoryString(communication, m_groupAttributeIDsCommunication)));
}

void CorpusExplorerTreeNodeCorpus::communicationDeleted(QString communicationID)
{
    m_nodeCommunications->removeItem(communicationID);
}

void CorpusExplorerTreeNodeCorpus::speakerAdded(QPointer<CorpusSpeaker> speaker)
{
    if (!speaker) return;
    CorpusExplorerTreeNodeSpeaker *nodeSpk = new CorpusExplorerTreeNodeSpeaker(speaker);
    m_nodeSpeakers->addNode(nodeSpk, QtilitiesCategory(categoryString(speaker, m_groupAttributeIDsSpeaker)));
}

void CorpusExplorerTreeNodeCorpus::speakerDeleted(QString speakerID)
{
    m_nodeSpeakers->removeItem(speakerID);
}

void CorpusExplorerTreeNodeCorpus::clear()
{
    if (m_nodeCommunications) {
        m_nodeCommunications->deleteAll();
        delete m_nodeCommunications;
    }
    m_nodeCommunications = 0;
    if (m_nodeSpeakers) {
        m_nodeSpeakers->deleteAll();
        delete m_nodeSpeakers;
    }
    m_nodeSpeakers = 0;
}

void CorpusExplorerTreeNodeCorpus::refresh() {
    buildTree();
}

void CorpusExplorerTreeNodeCorpus::buildTree()
{
    if (!m_corpus) return;
    clear();
    m_nodeCommunications = new TreeNode(tr("Communications"), this);
    m_nodeCommunications->enableCategorizedDisplay();
    m_nodeCommunications->setIcon(QIcon(qti_icon_FOLDER_16X16));
    m_nodeSpeakers = new TreeNode(tr("Speakers"), this);
    m_nodeSpeakers->enableCategorizedDisplay();
    m_nodeSpeakers->setIcon(QIcon(qti_icon_FOLDER_16X16));
    this->setName(m_corpus->ID());

    m_nodeCommunications->startProcessingCycle();
    foreach(QPointer<CorpusCommunication> com, m_corpus->communications()) {
        if (!com) continue;
        CorpusExplorerTreeNodeCommunication *nodeCom = new CorpusExplorerTreeNodeCommunication(com);
        nodeCom->startProcessingCycle();
        foreach (CorpusRecording *rec, com->recordings()) {
            CorpusExplorerTreeNodeRecording *nodeRec = new CorpusExplorerTreeNodeRecording(rec);
            nodeCom->addNode(nodeRec);
        }
        foreach (CorpusAnnotation *annot, com->annotations()) {
            CorpusExplorerTreeNodeAnnotation *nodeAnnot = new CorpusExplorerTreeNodeAnnotation(annot);
            nodeCom->addNode(nodeAnnot);
        }
        nodeCom->endProcessingCycle();
        m_nodeCommunications->addNode(nodeCom);
    }
    m_nodeCommunications->endProcessingCycle();
    m_nodeSpeakers->startProcessingCycle();
    foreach(QPointer<CorpusSpeaker> spk, m_corpus->speakers()) {
        if (!spk) continue;
        CorpusExplorerTreeNodeSpeaker *nodeSpk = new CorpusExplorerTreeNodeSpeaker(spk);
        m_nodeSpeakers->addNode(nodeSpk);
    }
    m_nodeSpeakers->endProcessingCycle();
    this->addNode(m_nodeCommunications);
    this->addNode(m_nodeSpeakers);
}

QString CorpusExplorerTreeNodeCorpus::categoryString(CorpusObject *obj, QStringList groupAttributeIDs)
{
    if (!obj) return QString();
    QString ret;
    foreach (QString ID, groupAttributeIDs) {
        QString s = obj->property(ID).toString().replace(":", " ");
        if (s.isEmpty()) s = "(NA)";
        ret.append(s).append("::");
    }
    if (ret.length() > 2) ret.chop(2);
    return ret;
}

void CorpusExplorerTreeNodeCorpus::setCommunicationsGrouping(QStringList groupAttributeIDs)
{
    m_nodeCommunications->startProcessingCycle();
    foreach (QObject *obj, m_nodeCommunications->treeChildren()) {
        CorpusExplorerTreeNodeCommunication *nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(obj);
        if (nodeCom) {
            CorpusCommunication *com = nodeCom->communication;
            nodeCom->setCategory(QtilitiesCategory(categoryString(com, groupAttributeIDs)));
        }
    }
    m_nodeCommunications->endProcessingCycle();
    m_groupAttributeIDsCommunication = groupAttributeIDs;
}

void CorpusExplorerTreeNodeCorpus::setSpeakersGrouping(QStringList groupAttributeIDs)
{
    m_nodeSpeakers->startProcessingCycle();
    foreach (QObject *obj, m_nodeSpeakers->treeChildren()) {
        CorpusExplorerTreeNodeSpeaker *nodeSpk = qobject_cast<CorpusExplorerTreeNodeSpeaker *>(obj);
        if (nodeSpk) {
            CorpusSpeaker *spk = nodeSpk->speaker;
            nodeSpk->setCategory(QtilitiesCategory(categoryString(spk, groupAttributeIDs)));
        }
    }
    m_nodeSpeakers->endProcessingCycle();
    m_groupAttributeIDsSpeaker = groupAttributeIDs;
}

// ==============================================================================================================================
// Communication tree node
// ==============================================================================================================================

CorpusExplorerTreeNodeCommunication::CorpusExplorerTreeNodeCommunication(QPointer<CorpusCommunication> com) :
    TreeNode(com->ID()), communication(com)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus_communication.png"));
    connect(communication, SIGNAL(corpusRecordingAdded(QPointer<Praaline::Core::CorpusRecording>)),
            this, SLOT(recordingAdded(QPointer<Praaline::Core::CorpusRecording>)));
    connect(communication, SIGNAL(corpusRecordingDeleted(QString, QString)),
            this, SLOT(recordingDeleted(QString,QString)));
    connect(communication, SIGNAL(corpusAnnotationAdded(QPointer<Praaline::Core::CorpusAnnotation>)),
            this, SLOT(annotationAdded(QPointer<Praaline::Core::CorpusAnnotation>)));
    connect(communication, SIGNAL(corpusAnnotationDeleted(QString, QString)),
            this, SLOT(annotationDeleted(QString,QString)));
}

void CorpusExplorerTreeNodeCommunication::recordingAdded(QPointer<CorpusRecording> recording)
{
    if (!recording) return;
    startProcessingCycle();
    CorpusExplorerTreeNodeRecording *nodeRec = new CorpusExplorerTreeNodeRecording(recording);
    addNode(nodeRec);
    endProcessingCycle();
}

void CorpusExplorerTreeNodeCommunication::recordingDeleted(QString communicationID, QString recordingID)
{
    Q_UNUSED(communicationID)
    removeItem(recordingID);
}

void CorpusExplorerTreeNodeCommunication::annotationAdded(QPointer<CorpusAnnotation> annotation)
{
    if (!annotation) return;
    startProcessingCycle();
    CorpusExplorerTreeNodeAnnotation *nodeAnnot = new CorpusExplorerTreeNodeAnnotation(annotation);
    addNode(nodeAnnot);
    endProcessingCycle();
}

void CorpusExplorerTreeNodeCommunication::annotationDeleted(QString communicationID, QString annotationID)
{
    Q_UNUSED(communicationID)
    removeItem(annotationID);
}

// ==============================================================================================================================
// Other tree nodes (speaker, recording, annotation)
// ==============================================================================================================================

CorpusExplorerTreeNodeSpeaker::CorpusExplorerTreeNodeSpeaker(QPointer<CorpusSpeaker> spk) :
    TreeNode(spk->ID()), speaker(spk)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus_speaker.png"));
}

CorpusExplorerTreeNodeRecording::CorpusExplorerTreeNodeRecording(QPointer<CorpusRecording> rec) :
    TreeNode(rec->ID()), recording(rec)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus_recording.png"));
}

CorpusExplorerTreeNodeAnnotation::CorpusExplorerTreeNodeAnnotation(QPointer<CorpusAnnotation> annot) :
    TreeNode(annot->ID()), annotation(annot)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus_annotation.png"));
}
