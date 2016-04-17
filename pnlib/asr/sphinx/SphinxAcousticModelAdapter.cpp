#include <QObject>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/corpus.h"
#include "pncore/annotation/annotationtiergroup.h"
#include "pncore/annotation/intervaltier.h"
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


