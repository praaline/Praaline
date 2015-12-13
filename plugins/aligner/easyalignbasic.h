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
    void setTiernamePhonetiszation(const QString &tiername) { m_tiernamePhonetisation = tiername; }
    void setPreciseUtteranceBoundaries(bool precise) { m_preciseUtteranceBoundaries = precise; }

    QString prepareAlignmentTextgrid(CorpusCommunication *com, QPointer<AnnotationTierGroup> tiers);
    void runEasyAlign(CorpusCommunication *com);
    void runSyllabify(CorpusCommunication *com);
    QString postAlignment(CorpusCommunication *com, QPointer<AnnotationTierGroup> tiers, bool fakeAlignment = false);
    QString mergeFiles(CorpusCommunication *com);

    QString quickScript(CorpusCommunication *com);

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
