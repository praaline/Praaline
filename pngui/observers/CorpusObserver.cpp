#include <QString>
#include <QVariant>
#include <QHash>
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
    CorpusExplorerTreeNodeCorpus *node = d->corpusNodes.value(corpus->ID(), Q_NULLPTR);
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

// ==============================================================================================================================
// Corpus tree node
// ==============================================================================================================================

CorpusExplorerTreeNodeCorpus::CorpusExplorerTreeNodeCorpus(const QString &corpusID) :
    TreeNode(corpusID), m_corpus(0)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus.png"));
    m_nodeCommunications = 0;
    m_nodeSpeakers = 0;
    this->setName(corpusID);
}

void CorpusExplorerTreeNodeCorpus::setCorpus(QPointer<Praaline::Core::Corpus> corpus)
{
    if (m_corpus) {
        // disconnect all signals from the previous corpus to this object
        disconnect(m_corpus, 0, this, 0);
    }
    m_corpus = corpus;
    buildTree();
    if (m_corpus) {
        connect(m_corpus, SIGNAL(communicationAdded(QPointer<Praaline::Core::CorpusCommunication>)),
                this, SLOT(communicationAdded(QPointer<Praaline::Core::CorpusCommunication>)));
        connect(m_corpus, SIGNAL(communicationDeleted(QString)),
                this, SLOT(communicationDeleted(QString)));
        connect(m_corpus, SIGNAL(speakerAdded(QPointer<Praaline::Core::CorpusSpeaker>)),
                this, SLOT(speakerAdded(QPointer<Praaline::Core::CorpusSpeaker>)));
        connect(m_corpus, SIGNAL(speakerDeleted(QString)),
                this, SLOT(speakerDeleted(QString)));
    }
}

void CorpusExplorerTreeNodeCorpus::communicationAdded(QPointer<CorpusCommunication> communication)
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

void CorpusExplorerTreeNodeCorpus::speakerAdded(QPointer<CorpusSpeaker> speaker)
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
    m_nodeCommunications = 0;
    if (m_nodeSpeakers) {
        m_nodeSpeakers->deleteAll();
        delete m_nodeSpeakers;
    }
    m_nodeSpeakers = 0;
}

void CorpusExplorerTreeNodeCorpus::buildTree()
{
    clear();
    if (!m_corpus) return;
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
