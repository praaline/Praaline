#ifndef IMPORTCORPUSITEMSWIZARDFINALPAGE_H
#define IMPORTCORPUSITEMSWIZARDFINALPAGE_H

#include <QWizardPage>
#include <QString>
#include <QMap>
#include <QSet>
#include <QPair>
#include <QMultiHash>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Interfaces/ImportAnnotations.h"
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
                                              QMap<QPair<QString, QString>, CorpusRecording *> &candidateRecordings,
                                              QMap<QPair<QString, QString>, CorpusAnnotation *> &candidateAnnotations,
                                              QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
                                              QSet<QString> &tierNamesCommon,
                                              QWidget *parent = nullptr);
    ~ImportCorpusItemsWizardFinalPage();

    void initializePage();
    bool validatePage();

private:
    Ui::ImportCorpusItemsWizardFinalPage *ui;
    ImportCorpusItemsWizardFinalPageData *d;

    void importPraat(CorpusCommunication *com, CorpusAnnotation *annot, QList<TierCorrespondance> &correspondances);
    void importTranscriber(CorpusCommunication *com, CorpusAnnotation *annot, QList<TierCorrespondance> &correspondances);
    void importSubRipTranscription(CorpusCommunication *com, CorpusAnnotation *annot, QList<TierCorrespondance> &correspondances);
};

#endif // IMPORTCORPUSITEMSWIZARDFINALPAGE_H
