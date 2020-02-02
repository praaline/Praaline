#ifndef EASYALIGNBASIC_H
#define EASYALIGNBASIC_H

#include <QObject>
#include <QString>

#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/IntervalTier.h"

#include "annotationpluginpraatscript.h"

class EasyAlignBasic : public AnnotationPluginPraatScript
{
    Q_OBJECT
public:
    explicit EasyAlignBasic(QObject *parent = nullptr);
    ~EasyAlignBasic();

    // Parameters
    void setAlignerOutputFilenameSuffix(const QString &suffix) { m_alignerOutputFilenameSuffix = suffix; }
    void setTiernameSelection(const QString &tiername) { m_tiernameSelection = tiername; }
    void setFilterSelection(const QString &filter) { m_filterSelection = filter; }
    void setTiernameTranscription(const QString &tiername) { m_tiernameTranscription = tiername; }
    void setTiernamePhonetisation(const QString &tiername) { m_tiernamePhonetisation = tiername; }
    void setPreciseUtteranceBoundaries(bool precise) { m_preciseUtteranceBoundaries = precise; }

    QString prepareAlignmentTextgrid(QList<Praaline::Core::Interval *> intervalsToAlign, Praaline::Core::IntervalTier *tier_ortho, QString filenameTextGrid);
    void runEasyAlign(QString filenameSound, QString filenameTextgrid);
    QString postAlignment(const QString &filenameTextgrid);
    void runSyllabify(QString filenameTextgrid);

    QString mergeFiles(Praaline::Core::CorpusCommunication *com);

    static QString runAllEasyAlignSteps(Praaline::Core::CorpusCommunication *com);

signals:

public slots:

private:
    QString m_alignerOutputFilenameSuffix;
    QString m_tiernameSelection;
    QString m_filterSelection;
    QString m_tiernameTranscription;
    QString m_tiernamePhonetisation;
    bool m_preciseUtteranceBoundaries;
};

#endif // EASYALIGNBASIC_H
