#ifndef SPHINXLANGUAGEMODELBUILDER_H
#define SPHINXLANGUAGEMODELBUILDER_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {

namespace Core {
class Corpus;
class CorpusCommunication;
class CorpusAnnotation;
}

namespace ASR {

struct SphinxLanguageModelBuilderData;

class PRAALINE_ASR_SHARED_EXPORT SphinxLanguageModelBuilder : public QObject
{
    Q_OBJECT
public:
    explicit SphinxLanguageModelBuilder(QObject *parent = nullptr);
    virtual ~SphinxLanguageModelBuilder();

    QString utterancesTierLevelID() const;
    QString utterancesTierAttributeID() const;
    void setUtterancesTier(const QString &levelID, const QString &attributeID);
    QString tokensTierLevelID() const;
    QString tokensTierAnnotationID() const;
    void setTokensTier(const QString &levelID, const QString &attributeID);

    QStringList speakersIncludeFilter() const;
    void setSpeakersIncludeFilter(const QStringList &speakerIDs);
    QStringList speakersExcludeFilter() const;
    void setSpeakersExcludeFilter(const QStringList &speakerIDs);

    int minimumNumberOfTokensInUtteranceFilter() const;
    void setMinimumNumberOfTokensInUtteranceFilter(int min);

    QStringList getNormalisedUtterances(Praaline::Core::CorpusAnnotation *annotation);

signals:

public slots:

private:
    SphinxLanguageModelBuilderData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXLANGUAGEMODELBUILDER_H
