#include <QDebug>
#include <QObject>
#include <QString>
#include "CorpusObject.h"
#include "Corpus.h"
#include "CorpusRecording.h"
#include "CorpusAnnotation.h"
#include "CorpusCommunication.h"
#include "datastore/CorpusRepository.h"
#include "datastore/MetadataDatastore.h"

namespace Praaline {
namespace Core {

CorpusCommunication::CorpusCommunication(CorpusRepository *repository, QObject *parent) :
    CorpusObject(repository, parent)
{
}

CorpusCommunication::CorpusCommunication(const QString &ID, CorpusRepository *repository, QObject *parent) :
    CorpusObject(ID, repository, parent)
{
}

CorpusCommunication::CorpusCommunication(CorpusCommunication *other, QObject *parent) :
    CorpusObject(0, parent)
{
    if (!other) return;
    m_ID = other->m_ID;
    m_originalID = other->m_originalID;
    m_name = other->m_name;
    m_corpusID = other->m_corpusID;
    m_repository = other->m_repository;
    copyPropertiesFrom(other);
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

void CorpusCommunication::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        m_isDirty = true;
    }
}

void CorpusCommunication::setCorpusID(const QString &corpusID)
{
    if (m_corpusID != corpusID) {
        m_corpusID = corpusID;
        foreach (QPointer<CorpusRecording> rec, m_recordings) if (rec) rec->setCorpusID(corpusID);
        foreach (QPointer<CorpusAnnotation> annot, m_annotations) if (annot) annot->setCorpusID(corpusID);
        m_isDirty = true;
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

QList<QString> CorpusCommunication::recordingIDs() const
{
    return m_recordings.keys();
}

void CorpusCommunication::addRecording(CorpusRecording *recording)
{
    if (!recording) return;
    recording->setParent(this);
    recording->setCorpusID(this->corpusID());
    m_recordings.insert(recording->ID(), recording);
    connect(recording, SIGNAL(changedID(QString,QString)), this, SLOT(recordingChangedID(QString,QString)));
    m_isDirty = true;
    emit corpusRecordingAdded(recording);
}

void CorpusCommunication::removeRecording(const QString &recordingID)
{
    if (!m_recordings.contains(recordingID)) return;
    QPointer<CorpusRecording> recording = m_recordings.value(recordingID);
    // remove recording
    m_recordings.remove(recordingID);
    delete recording;
    m_isDirty = true;
    deletedRecordingIDs << recordingID;
    emit corpusRecordingDeleted(this->ID(), recordingID);
}

//void CorpusCommunication::unlinkRecording(const QString &recordingID)
//{
//    if (!m_recordings.contains(recordingID)) return;
//    QPointer<CorpusRecording> recording = m_recordings.value(recordingID);
//    m_recordings.remove(recordingID);
//    recording->setNew(true);
//    m_isDirty = true;
//    deletedRecordingIDs << recordingID;
//    emit corpusRecordingDeleted(this->ID(), recordingID);
//}

void CorpusCommunication::recordingChangedID(const QString &oldID, const QString &newID)
{
    if (oldID == newID) return;
    QPointer<CorpusRecording> recording = m_recordings.value(oldID);
    if (!recording) return;
    m_recordings.remove(oldID);
    m_recordings.insert(newID, recording);
}

double CorpusCommunication::durationSec() const
{
    double duration(0);
    foreach (QPointer<CorpusRecording> rec, m_recordings) {
        if (!rec) continue;
        if (rec->duration().toDouble() > duration) duration = rec->duration().toDouble();
    }
    return duration;
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

QList<QString> CorpusCommunication::annotationIDs() const
{
    return m_annotations.keys();
}

void CorpusCommunication::addAnnotation(CorpusAnnotation *annotation)
{
    if (!annotation) return;
    annotation->setParent(this);
    annotation->setCorpusID(this->corpusID());
    m_annotations.insert(annotation->ID(), annotation);
    connect(annotation, SIGNAL(changedID(QString,QString)), this, SLOT(annotationChangedID(QString,QString)));
    m_isDirty = true;
    emit corpusAnnotationAdded(annotation);
}

void CorpusCommunication::removeAnnotation(const QString &annotationID)
{
    if (!m_annotations.contains(annotationID)) return;
    QPointer<CorpusAnnotation> annotation = m_annotations.value(annotationID);
    // remove annotation
    m_annotations.remove(annotationID);
    delete annotation;
    m_isDirty = true;
    deletedAnnotationIDs << annotationID;
    emit corpusAnnotationDeleted(this->ID(), annotationID);
}

//void CorpusCommunication::unlinkAnnotation(const QString &annotationID)
//{
//    if (!m_annotations.contains(annotationID)) return;
//    QPointer<CorpusAnnotation> annotation = m_annotations.value(annotationID);
//    m_annotations.remove(annotationID);
//    annotation->setNew(true);
//    m_isDirty = true;
//    deletedAnnotationIDs << annotationID;
//    emit corpusAnnotationDeleted(this->ID(), annotationID);
//}

void CorpusCommunication::annotationChangedID(const QString &oldID, const QString &newID)
{
    if (oldID == newID) return;
    QPointer<CorpusAnnotation> annotation = m_annotations.value(oldID);
    if (!annotation) return;
    m_annotations.remove(oldID);
    m_annotations.insert(newID, annotation);
}

bool CorpusCommunication::save() {
    if (!m_repository) return false;
    if (!m_repository->metadata()) return false;
    return m_repository->metadata()->saveCommunications(QList<QPointer<CorpusCommunication> >() << this);
}

} // namespace Core
} // namespace Praaline
