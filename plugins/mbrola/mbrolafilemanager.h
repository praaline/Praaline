#ifndef MBROLAFILEMANAGER_H
#define MBROLAFILEMANAGER_H

#include <QString>
#include <QList>

#include "annotationpluginpraatscript.h"
#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"

class MBROLAFileManager : public AnnotationPluginPraatScript
{
public:
    MBROLAFileManager();

    void extractPhoParameters(QPointer<Praaline::Core::CorpusRecording> rec, QPointer<Praaline::Core::CorpusAnnotation> annot,
                              const QString &attributeForPhoneme);
    bool updatePhoneTierFromPhoFile(const QString &filenamePho, Praaline::Core::IntervalTier *tier,
                                    const QString &attributeForPhoneme, bool updatePhone = false, bool updatePitch = false);
    bool createPhoFile(const QString &filenamePho, Praaline::Core::IntervalTier *tier_phones, const QString &attributeID = QString());
    bool createPhoFile(const QString &filenamePho, QList<Praaline::Core::Interval *> phones, const QString &attributeID = QString());
};

#endif // MBROLAFILEMANAGER_H
