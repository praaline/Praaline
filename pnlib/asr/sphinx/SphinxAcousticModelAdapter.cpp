#include <QObject>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pnlib/mediautil/AudioSegmenter.h"

#include "SphinxAcousticModelAdapter.h"

struct SphinxAcousticModelAdapterData {
    QString adaptationPath;
    QString tiernameUtterances;
    QString tiernameTokens;
    QMultiHash<QString, QString> dictionary;
};

SphinxAcousticModelAdapter::SphinxAcousticModelAdapter(QObject *parent)
    : QObject(parent), d(new SphinxAcousticModelAdapterData)
{
    d->tiernameUtterances = "segment";
    d->tiernameTokens = "tok_min";
}

SphinxAcousticModelAdapter::~SphinxAcousticModelAdapter()
{
    delete d;
}


