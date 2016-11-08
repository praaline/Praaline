#ifndef CORPUSCOMMUNICATION_H
#define CORPUSCOMMUNICATION_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include <QMap>
#include "CorpusObject.h"
#include "CorpusRecording.h"
#include "CorpusAnnotation.h"

namespace Praaline {
namespace Core {

class Corpus;

class PRAALINE_CORE_SHARED_EXPORT CorpusCommunication : public CorpusObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(double durationSec READ durationSec)

public:
    explicit CorpusCommunication(QObject *parent = 0);
    CorpusCommunication(const QString &ID, QObject *parent = 0);
    CorpusCommunication(CorpusCommunication *other, QObject *parent = 0);
    ~CorpusCommunication();

    CorpusObject::Type type() const { return CorpusObject::Type_Communication; }

    QPointer<Corpus> corpus() const;
    QString basePath() const;
    QString baseMediaPath() const;

    // Basic data
    QString name() const { return m_name; }
    void setName(const QString &name);

    void setCorpusID(const QString &corpusID);

    // RECORDINGS
    QPointer<CorpusRecording> recording(const QString &recordingID) const;
    int recordingsCount() const;
    bool hasRecordings() const;
    bool hasRecording(const QString &recordingID) const;
    const QMap<QString, QPointer<CorpusRecording> > &recordings() const;
    QList<QString> recordingIDs() const;
    void addRecording(CorpusRecording *recording);
    void removeRecording(const QString &recordingID);
    void unlinkRecording(const QString &recordingID);
    double durationSec() const;

    // ANNOTATIONS
    QPointer<CorpusAnnotation> annotation(const QString &annotationID) const;
    int annotationsCount() const;
    bool hasAnnotations() const;
    bool hasAnnotation(const QString &annotationID) const;
    const QMap<QString, QPointer<CorpusAnnotation> > &annotations() const;
    QList<QString> annotationIDs() const;
    void addAnnotation(CorpusAnnotation *annotation);
    void removeAnnotation(const QString &annotationID);
    void unlinkAnnotation(const QString &annotationID);

    // Database helpers
    QList<QString> deletedAnnotationIDs;
    QList<QString> deletedRecordingIDs;

signals:
    void corpusRecordingAdded(QPointer<Praaline::Core::CorpusRecording> recording);
    void corpusRecordingDeleted(QString communicationID, QString recordingID);
    void corpusAnnotationAdded(QPointer<Praaline::Core::CorpusAnnotation> annotation);
    void corpusAnnotationDeleted(QString communicationID, QString annotationID);

public slots:

private slots:
    void recordingChangedID(const QString &oldID, const QString &newID);
    void annotationChangedID(const QString &oldID, const QString &newID);
    
protected:
    QString m_name;
    QMap<QString, QPointer<CorpusRecording> > m_recordings;
    QMap<QString, QPointer<CorpusAnnotation> > m_annotations;
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSITEM_H
