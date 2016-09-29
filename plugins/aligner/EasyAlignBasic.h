#ifndef EASYALIGNBASIC_H
#define EASYALIGNBASIC_H

#include <QObject>
#include <QString>
#include "pncore/corpus/corpuscommunication.h"
#include "pncore/annotation/annotationtiergroup.h"
#include "annotationpluginpraatscript.h"

class EasyAlignBasic : public AnnotationPluginPraatScript
{
    Q_OBJECT
public:
    explicit EasyAlignBasic(QObject *parent = 0);
    ~EasyAlignBasic();

    // Parameters
    void setAlignerOutputFilenameSuffix(const QString &suffix) { m_alignerOutputFilenameSuffix = suffix; }
    void setTiernameSelection(const QString &tiername) { m_tiernameSelection = tiername; }
    void setFilterSelection(const QString &filter) { m_filterSelection = filter; }
    void setTiernameTranscription(const QString &tiername) { m_tiernameTranscription = tiername; }
    void setTiernamePhonetisation(const QString &tiername) { m_tiernamePhonetisation = tiername; }
    void setPreciseUtteranceBoundaries(bool precise) { m_preciseUtteranceBoundaries = precise; }

    QString prepareAlignmentTextgrid(QList<Interval *> intervalsToAlign, IntervalTier *tier_ortho, QString filenameTextGrid);
    void runEasyAlign(QString filenameSound, QString filenameTextgrid);
    QString postAlignment(const QString &filenameTextgrid);
    void runSyllabify(QString filenameTextgrid);

    QString mergeFiles(CorpusCommunication *com);

    static QString runAllEasyAlignSteps(Corpus *corpus, CorpusCommunication *com);

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
