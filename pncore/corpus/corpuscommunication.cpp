#include <QDebug>
#include <QObject>
#include <QString>
#include "corpusobject.h"
#include "corpus.h"
#include "corpusrecording.h"
#include "corpusannotation.h"
#include "corpuscommunication.h"

CorpusCommunication::CorpusCommunication(QObject *parent) :
    CorpusObject(parent)
{
}

CorpusCommunication::CorpusCommunication(const QString &ID, QObject *parent) :
    CorpusObject(ID, parent)
{
}

CorpusCommunication::CorpusCommunication(CorpusCommunication *other, QObject *parent) :
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

CorpusCommunication::~CorpusCommunication()
{
    qDeleteAll(m_recordings);
    qDeleteAll(m_annotations);
}

QPointer<Corpus> CorpusCommunication::corpus() const
{
    return qobject_cast<Corpus *>(this->parent());
}

QString CorpusCommunication::basePath() const
{
    Corpus *myCorpus = qobject_cast<Corpus *>(this->parent());
    if (myCorpus)
        return myCorpus->basePath();
    else
        return QString();
}

void CorpusCommunication::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        setDirty(true);
    }
}

void CorpusCommunication::setCorpusID(const QString &corpusID)
{
    if (m_corpusID != corpusID) {
        m_corpusID = corpusID;
        foreach (QPointer<CorpusRecording> rec, m_recordings) if (rec) rec->setCorpusID(corpusID);
        foreach (QPointer<CorpusAnnotation> annot, m_annotations) if (annot) annot->setCorpusID(corpusID);
        setDirty(true);
    }
}

// ==========================================================================================================
// ITEM MANAGEMENT
// ==========================================================================================================

// ==========================================================================================================
// Recordings
// ==========================================================================================================

QPointer<CorpusRecording> CorpusCommunication::recording(const QString &recordingID) const
{
    return m_recordings.value(recordingID, 0);
}

int CorpusCommunication::recordingsCount() const
{
    return m_recordings.count();
}

bool CorpusCommunication::hasRecordings() const
{
    return !m_recordings.isEmpty();
}

bool CorpusCommunication::hasRecording(const QString &recordingID) const
{
    return m_recordings.contains(recordingID);
}

const QMap<QString, QPointer<CorpusRecording> > &CorpusCommunication::recordings() const
{
    return m_recordings;
}

void CorpusCommunication::addRecording(CorpusRecording *recording)
{
    if (!recording) return;
    recording->setParent(this);
    recording->setCorpusID(this->corpusID());
    m_recordings.insert(recording->ID(), recording);
    connect(recording, SIGNAL(changedID(QString,QString)), this, SLOT(recordingChangedID(QString,QString)));
    setDirty(true);
    emit corpusRecordingAdded(recording);
}

void CorpusCommunication::removeRecording(const QString &recordingID)
{
    if (!m_recordings.contains(recordingID)) return;
    QPointer<CorpusRecording> recording = m_recordings.value(recordingID);
    // remove recording
    m_recordings.remove(recordingID);
    delete recording;
    setDirty(true);
    deletedRecordingIDs << recordingID;
    emit corpusRecordingDeleted(this->ID(), recordingID);
}

void CorpusCommunication::unlinkRecording(const QString &recordingID)
{
    if (!m_recordings.contains(recordingID)) return;
    QPointer<CorpusRecording> recording = m_recordings.value(recordingID);
    m_recordings.remove(recordingID);
    recording->setNew(true);
    setDirty(true);
    deletedRecordingIDs << recordingID;
    emit corpusRecordingDeleted(this->ID(), recordingID);
}

void CorpusCommunication::recordingChangedID(const QString &oldID, const QString &newID)
{
    if (oldID == newID) return;
    QPointer<CorpusRecording> recording = m_recordings.value(oldID);
    if (!recording) return;
    m_recordings.remove(oldID);
    m_recordings.insert(newID, recording);
}

// ==========================================================================================================
// Annotations
// ==========================================================================================================

QPointer<CorpusAnnotation> CorpusCommunication::annotation(const QString &annotationID) const
{
    return m_annotations.value(annotationID, 0);
}

int CorpusCommunication::annotationsCount() const
{
    return m_annotations.count();
}

bool CorpusCommunication::hasAnnotations() const
{
    return !m_annotations.isEmpty();
}

bool CorpusCommunication::hasAnnotation(const QString &annotationID) const
{
    return m_annotations.contains(annotationID);
}

const QMap<QString, QPointer<CorpusAnnotation> > &CorpusCommunication::annotations() const
{
    return m_annotations;
}

void CorpusCommunication::addAnnotation(CorpusAnnotation *annotation)
{
    if (!annotation) return;
    annotation->setParent(this);
    annotation->setCorpusID(this->corpusID());
    m_annotations.insert(annotation->ID(), annotation);
    connect(annotation, SIGNAL(changedID(QString,QString)), this, SLOT(annotationChangedID(QString,QString)));
    setDirty(true);
    emit corpusAnnotationAdded(annotation);
}

void CorpusCommunication::removeAnnotation(const QString &annotationID)
{
    if (!m_annotations.contains(annotationID)) return;
    QPointer<CorpusAnnotation> annotation = m_annotations.value(annotationID);
    // remove annotation
    m_annotations.remove(annotationID);
    delete annotation;
    setDirty(true);
    deletedAnnotationIDs << annotationID;
    emit corpusAnnotationDeleted(this->ID(), annotationID);
}

void CorpusCommunication::unlinkAnnotation(const QString &annotationID)
{
    if (!m_annotations.contains(annotationID)) return;
    QPointer<CorpusAnnotation> annotation = m_annotations.value(annotationID);
    m_annotations.remove(annotationID);
    annotation->setNew(true);
    setDirty(true);
    deletedAnnotationIDs << annotationID;
    emit corpusAnnotationDeleted(this->ID(), annotationID);
}

void CorpusCommunication::annotationChangedID(const QString &oldID, const QString &newID)
{
    if (oldID == newID) return;
    QPointer<CorpusAnnotation> annotation = m_annotations.value(oldID);
    if (!annotation) return;
    m_annotations.remove(oldID);
    m_annotations.insert(newID, annotation);
}
