#ifndef IMPORTCORPUSITEMSWIZARDFINALPAGE_H
#define IMPORTCORPUSITEMSWIZARDFINALPAGE_H

#include <QWizardPage>
#include <QString>
#include <QMap>
#include <QSet>
#include <QPair>
#include <QMultiHash>
#include "ImportAnnotations.h"

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class ImportCorpusItemsWizardFinalPage;
}

class ImportCorpusItemsWizardFinalPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizardFinalPage(QPointer<Corpus> corpus,
                                              QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &candidateRecordings,
                                              QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &candidateAnnotations,
                                              QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
                                              QSet<QString> &tierNamesCommon,
                                              QWidget *parent = 0);
    ~ImportCorpusItemsWizardFinalPage();

    void initializePage();
    bool validatePage();

private:
    Ui::ImportCorpusItemsWizardFinalPage *ui;

    QPointer<Corpus> m_corpus;
    QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &m_candidateRecordings;
    QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &m_candidateAnnotations;
    QMultiHash<QString, TierCorrespondance> &m_tierCorrespondances;
    QSet<QString> &m_tierNamesCommon;

    void importPraat(QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot, QList<TierCorrespondance> &correspondances);
    void importTranscriber(QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot, QList<TierCorrespondance> &correspondances);
    void importSubRipTranscription(QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot, QList<TierCorrespondance> &correspondances);
};

#endif // IMPORTCORPUSITEMSWIZARDFINALPAGE_H