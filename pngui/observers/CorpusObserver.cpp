#include <QString>
#include <QVariant>
#include <QHash>
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusRecording.h"
#include "PraalineCore/Corpus/CorpusAnnotation.h"
#include "PraalineCore/Corpus/CorpusSpeaker.h"
#include "PraalineCore/Corpus/CorpusObjectInfo.h"
using namespace Praaline::Core;

#include "CorpusObserver.h"

// ==============================================================================================================================
// Corpus observer: lists all corpora in a repository
// ==============================================================================================================================

struct CorpusObserverData {
    QPointer<Praaline::Core::CorpusRepository> repository;
    TreeNode *nodeRepository;
    QHash<QString, QPointer<CorpusExplorerTreeNodeCorpus> > corpusNodes;
};

CorpusObserver::CorpusObserver(QPointer<Praaline::Core::CorpusRepository> repository, QObject *parent) :
    QObject(parent), d(new CorpusObserverData)
{
    d->repository = repository;
    d->nodeRepository = new TreeNode(d->repository->ID(), this);
    foreach (CorpusObjectInfo item, d->repository->listCorporaInfo()) {
        QString corpusID = item.attribute("corpusID").toString();
        if (corpusID.isEmpty()) continue;
        CorpusExplorerTreeNodeCorpus *node = new CorpusExplorerTreeNodeCorpus(corpusID);
        d->corpusNodes.insert(corpusID, node);
        d->nodeRepository->addNode(node);
    }
}

CorpusObserver::~CorpusObserver()
{
    delete d;
}

QPointer<CorpusRepository> CorpusObserver::repository() const
{
    return d->repository;
}

TreeNode *CorpusObserver::nodeRepository() const
{
    return d->nodeRepository;
}

void CorpusObserver::addCorpus(QPointer<Corpus> corpus)
{
    if (!corpus) return;
    CorpusExplorerTreeNodeCorpus *node = d->corpusNodes.value(corpus->ID(), nullptr);
    if (!node) {
        node = new CorpusExplorerTreeNodeCorpus(corpus->ID());
        d->corpusNodes.insert(corpus->ID(), node);
        d->nodeRepository->addNode(node);
    }
    node->setCorpus(corpus);
}

void CorpusObserver::removeCorpus(QString corpusID)
{
    d->nodeRepository->removeItem(corpusID);
}

void CorpusObserver::setCommunicationsGrouping(QStringList groupAttributeIDs)
{
    foreach (QObject *obj, d->nodeRepository->treeChildren()) {
        CorpusExplorerTreeNodeCorpus *node = qobject_cast<CorpusExplorerTreeNodeCorpus *>(obj);
        if (node) node->setCommunicationsGrouping(groupAttributeIDs);
    }
}

void CorpusObserver::setSpeakersGrouping(QStringList groupAttributeIDs)
{
    foreach (QObject *obj, d->nodeRepository->treeChildren()) {
        CorpusExplorerTreeNodeCorpus *node = qobject_cast<CorpusExplorerTreeNodeCorpus *>(obj);
        if (node) node->setSpeakersGrouping(groupAttributeIDs);
    }
}

void CorpusObserver::refresh()
{
    foreach (QPointer<CorpusExplorerTreeNodeCorpus> node, d->corpusNodes.values()) {
        if (node) node->refresh();
    }
}

// ==============================================================================================================================
// Corpus tree node
// ==============================================================================================================================

CorpusExplorerTreeNodeCorpus::CorpusExplorerTreeNodeCorpus(const QString &corpusID) :
    TreeNode(corpusID), m_corpus(nullptr)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus.png"));
    m_nodeCommunications = nullptr;
    m_nodeSpeakers = nullptr;
    this->setName(corpusID);
}

void CorpusExplorerTreeNodeCorpus::setCorpus(QPointer<Praaline::Core::Corpus> corpus)
{
    if (m_corpus) {
        // disconnect all signals from the previous corpus to this object
        disconnect(m_corpus, nullptr, this, nullptr);
    }
    m_corpus = corpus;
    buildTree();
    if (m_corpus) {
        connect(m_corpus.data(), &Corpus::communicationAdded,
                this, &CorpusExplorerTreeNodeCorpus::communicationAdded);
        connect(m_corpus.data(), &Corpus::communicationDeleted,
                this, &CorpusExplorerTreeNodeCorpus::communicationDeleted);
        connect(m_corpus.data(), &Corpus::speakerAdded,
                this, &CorpusExplorerTreeNodeCorpus::speakerAdded);
        connect(m_corpus.data(), &Corpus::speakerDeleted,
                this, &CorpusExplorerTreeNodeCorpus::speakerDeleted);
    }
}

void CorpusExplorerTreeNodeCorpus::communicationAdded(CorpusCommunication *communication)
{
    if (!communication) return;
    if (!m_nodeCommunications) return;
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
    if (!m_nodeCommunications) return;
    m_nodeCommunications->removeItem(communicationID);
}

void CorpusExplorerTreeNodeCorpus::speakerAdded(CorpusSpeaker *speaker)
{
    if (!speaker) return;
    if (!m_nodeSpeakers) return;
    CorpusExplorerTreeNodeSpeaker *nodeSpk = new CorpusExplorerTreeNodeSpeaker(speaker);
    m_nodeSpeakers->addNode(nodeSpk, QtilitiesCategory(categoryString(speaker, m_groupAttributeIDsSpeaker)));
}

void CorpusExplorerTreeNodeCorpus::speakerDeleted(QString speakerID)
{
    if (!m_nodeSpeakers) return;
    m_nodeSpeakers->removeItem(speakerID);
}

void CorpusExplorerTreeNodeCorpus::clear()
{
    if (m_nodeCommunications) {
        m_nodeCommunications->deleteAll();
        delete m_nodeCommunications;
    }
    m_nodeCommunications = nullptr;
    if (m_nodeSpeakers) {
        m_nodeSpeakers->deleteAll();
        delete m_nodeSpeakers;
    }
    m_nodeSpeakers = nullptr;
}

void CorpusExplorerTreeNodeCorpus::refresh()
{
    buildTree();
}

void CorpusExplorerTreeNodeCorpus::buildTree()
{
    clear();
    if (!m_corpus) return;

    startProcessingCycle();
    m_nodeCommunications = new TreeNode(tr("Communications"), this);
    m_nodeCommunications->enableCategorizedDisplay();
    m_nodeCommunications->setIcon(QIcon(qti_icon_FOLDER_16X16));
    m_nodeSpeakers = new TreeNode(tr("Speakers"), this);
    m_nodeSpeakers->enableCategorizedDisplay();
    m_nodeSpeakers->setIcon(QIcon(qti_icon_FOLDER_16X16));
    this->setName(m_corpus->ID());

    m_nodeCommunications->startProcessingCycle();
    foreach(CorpusCommunication *com, m_corpus->communications()) {
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
    endProcessingCycle();
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
    if (!m_nodeCommunications) return;
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
    if (!m_nodeSpeakers) return;
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

CorpusExplorerTreeNodeCommunication::CorpusExplorerTreeNodeCommunication(CorpusCommunication *com) :
    TreeNode(com->ID()), communication(com)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus_communication.png"));
    connect(communication, &CorpusCommunication::corpusRecordingAdded,
            this, &CorpusExplorerTreeNodeCommunication::recordingAdded);
    connect(communication, &CorpusCommunication::corpusRecordingDeleted,
            this, &CorpusExplorerTreeNodeCommunication::recordingDeleted);
    connect(communication, &CorpusCommunication::corpusAnnotationAdded,
            this, &CorpusExplorerTreeNodeCommunication::annotationAdded);
    connect(communication, &CorpusCommunication::corpusAnnotationDeleted,
            this, &CorpusExplorerTreeNodeCommunication::annotationDeleted);
}

void CorpusExplorerTreeNodeCommunication::recordingAdded(CorpusRecording *recording)
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

void CorpusExplorerTreeNodeCommunication::annotationAdded(CorpusAnnotation *annotation)
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

CorpusExplorerTreeNodeRecording::CorpusExplorerTreeNodeRecording(CorpusRecording *rec) :
    TreeNode(rec->ID()), recording(rec)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus_recording.png"));
}

CorpusExplorerTreeNodeAnnotation::CorpusExplorerTreeNodeAnnotation(CorpusAnnotation *annot) :
    TreeNode(annot->ID()), annotation(annot)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus_annotation.png"));
}
