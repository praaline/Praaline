#include <QObject>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "PraalineMedia/AudioSegmenter.h"
using namespace Praaline::Media;

#include "PraalineASR/Sphinx/SphinxAcousticModelAdapter.h"

namespace Praaline {
namespace ASR {

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

} // namespace ASR
} // namespace Praaline

