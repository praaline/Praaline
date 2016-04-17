#ifndef SPHINXACOUSTICMODELTRAINER_H
#define SPHINXACOUSTICMODELTRAINER_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

class Corpus;
class CorpusCommunication;
class SphinxPronunciationDictionary;

struct SphinxAcousticModelTrainerData;

class SphinxAcousticModelTrainer : public QObject
{
    Q_OBJECT
public:
    explicit SphinxAcousticModelTrainer(QObject *parent = 0);
    virtual ~SphinxAcousticModelTrainer();

    void setTierUtterances(const QString &levelID, const QString &attributeID);
    void setTierTokens(const QString &levelID, const QString &attributeID);
    void setPronunciationDictionary(QPointer<SphinxPronunciationDictionary> dic);
    void setOutputPath(const QString &path);
    void setSpeakersIncludeFilter(const QStringList &speakerIDs);
    void setSpeakersExcludeFilter(const QStringList &speakerIDs);

    bool createFiles(QPointer<Corpus> corpus, QList<QPointer<CorpusCommunication> > &communications,
                     QStringList &outUnknownWordsList, bool splitTrainTest = false, bool createSoundSegments = false);

signals:
    void printMessage(QString message);


public slots:

private:
    SphinxAcousticModelTrainerData *d;
};

#endif // SPHINXACOUSTICMODELTRAINER_H
