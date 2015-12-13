#include "pncore/corpus/corpuscommunication.h"
#include "corpusobserver.h"

CorpusExplorerTreeNodeCommunication::CorpusExplorerTreeNodeCommunication(QPointer<CorpusCommunication> com) :
    TreeNode(com->ID()), communication(com)
{
    setIcon(QIcon(":/icons/corpusexplorer/corpus_communication.png"));
    connect(communication, SIGNAL(corpusRecordingAdded(QPointer<CorpusRecording>)), this, SLOT(recordingAdded(QPointer<CorpusRecording>)));
    connect(communication, SIGNAL(corpusRecordingDeleted(QString, QString)), this, SLOT(recordingDeleted(QString,QString)));
    connect(communication, SIGNAL(corpusAnnotationAdded(QPointer<CorpusAnnotation>)), this, SLOT(annotationAdded(QPointer<CorpusAnnotation>)));
    connect(communication, SIGNAL(corpusAnnotationDeleted(QString, QString)), this, SLOT(annotationDeleted(QString,QString)));
}

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

// --------------------------------------------------------------------------------------------------------------------

CorpusObserver::CorpusObserver(QPointer<Corpus> corpus, QObject *parent)
    : QObject(parent), m_corpus(corpus)
{
    m_nodeCommunications = 0;
    m_nodeSpeakers = 0;
    m_nodeCorpus = 0;
    buildTree();
    connect(m_corpus, SIGNAL(corpusCommunicationAdded(QPointer<CorpusCommunication>)),
            this, SLOT(communicationAdded(QPointer<CorpusCommunication>)));
    connect(m_corpus, SIGNAL(corpusCommunicationDeleted(QString)),
            this, SLOT(communicationDeleted(QString)));
    connect(m_corpus, SIGNAL(corpusSpeakerAdded(QPointer<CorpusSpeaker>)),
            this, SLOT(speakerAdded(QPointer<CorpusSpeaker>)));
    connect(m_corpus, SIGNAL(corpusSpeakerDeleted(QString)),
            this, SLOT(speakerDeleted(QString)));
}

CorpusObserver::~CorpusObserver()
{
}

void CorpusObserver::clear()
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
    if (m_nodeCorpus) {
        m_nodeCorpus->deleteAll();
        delete m_nodeCorpus;
    }
    m_nodeCorpus = 0;
}

void CorpusObserver::refresh() {
    buildTree();
}

void CorpusObserver::buildTree()
{
    if (!m_corpus) return;
    clear();
    m_nodeCommunications = new TreeNode(tr("Communications"), this);
    m_nodeCommunications->enableCategorizedDisplay();
    m_nodeCommunications->setIcon(QIcon(qti_icon_FOLDER_16X16));
    m_nodeSpeakers = new TreeNode(tr("Speakers"), this);
    m_nodeSpeakers->enableCategorizedDisplay();
    m_nodeSpeakers->setIcon(QIcon(qti_icon_FOLDER_16X16));
    m_nodeCorpus = new TreeNode(m_corpus->ID(), this);

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
    m_nodeCorpus->addNode(m_nodeCommunications);
    m_nodeCorpus->addNode(m_nodeSpeakers);
}

QString CorpusObserver::categoryString(CorpusObject *obj, QStringList groupAttributeIDs)
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

void CorpusObserver::setCommunicationsGrouping(QStringList groupAttributeIDs)
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

void CorpusObserver::setSpeakersGrouping(QStringList groupAttributeIDs)
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

void CorpusObserver::communicationAdded(QPointer<CorpusCommunication> communication)
{
    if (!communication) return;
    m_nodeCommunications->startProcessingCycle();
    CorpusExplorerTreeNodeCommunication *nodeCom = new CorpusExplorerTreeNodeCommunication(communication);
    nodeCom->startProcessingCycle();
    foreach (CorpusRecording *rec, communication->recordings()) {
        CorpusExplorerTreeNodeRecording *nodeRec = new CorpusExplorerTreeNodeRecording(rec);
        nodeCom->addNode(nodeRec);
    }
    foreach (CorpusAnnotation *annot, communication->annotations()) {
        CorpusExplorerTreeNodeAnnotation *nodeAnnot = new CorpusExplorerTreeNodeAnnotation(annot);
        nodeCom->addNode(nodeAnnot);
    }
    nodeCom->endProcessingCycle();
    m_nodeCommunications->addNode(nodeCom, QtilitiesCategory(categoryString(communication, m_groupAttributeIDsCommunication)));
    m_nodeCommunications->endProcessingCycle();
}

void CorpusObserver::communicationDeleted(QString communicationID)
{
    m_nodeCommunications->removeItem(communicationID);
}

void CorpusObserver::speakerAdded(QPointer<CorpusSpeaker> speaker)
{
    if (!speaker) return;
    m_nodeSpeakers->startProcessingCycle();
    CorpusExplorerTreeNodeSpeaker *nodeSpk = new CorpusExplorerTreeNodeSpeaker(speaker);
    m_nodeSpeakers->addNode(nodeSpk, QtilitiesCategory(categoryString(speaker, m_groupAttributeIDsSpeaker)));
    m_nodeSpeakers->endProcessingCycle();
}

void CorpusObserver::speakerDeleted(QString speakerID)
{
    m_nodeSpeakers->removeItem(speakerID);
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


