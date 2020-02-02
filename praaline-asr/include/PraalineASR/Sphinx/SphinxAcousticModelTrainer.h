#ifndef SPHINXACOUSTICMODELTRAINER_H
#define SPHINXACOUSTICMODELTRAINER_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {

namespace Core {
class Corpus;
class CorpusCommunication;
}

namespace ASR {

class SphinxPronunciationDictionary;
struct SphinxAcousticModelTrainerData;

class PRAALINE_ASR_SHARED_EXPORT SphinxAcousticModelTrainer : public QObject
{
    Q_OBJECT
public:
    explicit SphinxAcousticModelTrainer(QObject *parent = nullptr);
    virtual ~SphinxAcousticModelTrainer();

    void setTierUtterances(const QString &levelID, const QString &attributeID);
    void setTierTokens(const QString &levelID, const QString &attributeID);
    void setPronunciationDictionary(const SphinxPronunciationDictionary &dic);
    void setOutputPath(const QString &path);
    void setSpeakersIncludeFilter(const QStringList &speakerIDs);
    void setSpeakersExcludeFilter(const QStringList &speakerIDs);

    bool createFiles(QList<Praaline::Core::CorpusCommunication *> &communications,
                     QStringList &outUnknownWordsList, bool splitTrainTest = false, bool createSoundSegments = false);

signals:
    void printMessage(QString message);


public slots:

private:
    SphinxAcousticModelTrainerData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXACOUSTICMODELTRAINER_H
