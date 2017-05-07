#ifndef IMPORTCORPUSITEMSWIZARDFINALPAGE_H
#define IMPORTCORPUSITEMSWIZARDFINALPAGE_H

#include <QWizardPage>
#include <QString>
#include <QMap>
#include <QSet>
#include <QPair>
#include <QMultiHash>

#include "pncore/corpus/Corpus.h"
#include "pncore/interfaces/ImportAnnotations.h"
using namespace Praaline::Core;

namespace Ui {
class ImportCorpusItemsWizardFinalPage;
}

struct ImportCorpusItemsWizardFinalPageData;

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
    ImportCorpusItemsWizardFinalPageData *d;

    void importPraat(QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot, QList<TierCorrespondance> &correspondances);
    void importTranscriber(QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot, QList<TierCorrespondance> &correspondances);
    void importSubRipTranscription(QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot, QList<TierCorrespondance> &correspondances);
};

#endif // IMPORTCORPUSITEMSWIZARDFINALPAGE_H
