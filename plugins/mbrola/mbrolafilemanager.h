#ifndef MBROLAFILEMANAGER_H
#define MBROLAFILEMANAGER_H

#include <QString>
#include <QList>

#include "annotationpluginpraatscript.h"
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

class MBROLAFileManager : public AnnotationPluginPraatScript
{
public:
    MBROLAFileManager();

    void extractPhoParameters(Corpus *corpus, QPointer<CorpusRecording> rec, QPointer<CorpusAnnotation> annot,
                              const QString &attributeForPhoneme);
    bool updatePhoneTierFromPhoFile(const QString &filenamePho, QPointer<IntervalTier> tier,
                                    const QString &attributeForPhoneme, bool updatePhone = false, bool updatePitch = false);
    bool createPhoFile(const QString &filenamePho, QPointer<IntervalTier> tier_phones, const QString &attributeID = QString());
    bool createPhoFile(const QString &filenamePho, QList<Interval *> phones, const QString &attributeID = QString());
};

#endif // MBROLAFILEMANAGER_H
