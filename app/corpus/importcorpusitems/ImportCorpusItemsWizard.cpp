#include <QSet>
#include <QMap>
#include <QMultiHash>
#include "ImportCorpusItemsWizardSelectionPage.h"
#include "ImportCorpusItemsWizardProcessMediaPage.h"
#include "ImportCorpusItemsWizardAnalysePage.h"
#include "ImportCorpusItemsWizardCorrespondancesPage.h"
#include "ImportCorpusItemsWizardFinalPage.h"
#include "ImportAnnotations.h"

#include "ImportCorpusItemsWizard.h"
#include "ui_ImportCorpusItemsWizard.h"

struct ImportCorpusItemsWizardData {
    ImportCorpusItemsWizardData() :
        pageSelection(0), pageProcessMedia(0), pageAnalyse(0), pageCorrespondances(0), pageFinal(0),
        repository(0)
    {}

    // Pages
    ImportCorpusItemsWizardSelectionPage *pageSelection;
    ImportCorpusItemsWizardProcessMediaPage *pageProcessMedia;
    ImportCorpusItemsWizardAnalysePage *pageAnalyse;
    ImportCorpusItemsWizardCorrespondancesPage *pageCorrespondances;
    ImportCorpusItemsWizardFinalPage *pageFinal;

    // State
    QPointer<CorpusRepository> repository;
    QMap<QPair<QString, QString>, QPointer<CorpusRecording> > candidateRecordings;
    QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > candidateAnnotations;
    QMultiHash<QString, TierCorrespondance> tierCorrespondances;
    QSet<QString> tierNamesCommon;
};

ImportCorpusItemsWizard::ImportCorpusItemsWizard(QPointer<CorpusRepository> repository, QWidget *parent) :
    QWizard(parent), ui(new Ui::ImportCorpusItemsWizard)
{
    ui->setupUi(this);
    d = new ImportCorpusItemsWizardData;

    d->repository = repository;
    d->pageSelection = new ImportCorpusItemsWizardSelectionPage(d->repository, d->candidateRecordings, d->candidateAnnotations, this);
    d->pageProcessMedia = new ImportCorpusItemsWizardProcessMediaPage(d->candidateRecordings, this);
    d->pageAnalyse = new ImportCorpusItemsWizardAnalysePage(d->candidateAnnotations, d->tierCorrespondances, d->tierNamesCommon, this);
    d->pageCorrespondances = new ImportCorpusItemsWizardCorrespondancesPage(d->repository, d->tierCorrespondances, d->tierNamesCommon, this);
    d->pageFinal = new ImportCorpusItemsWizardFinalPage(d->repository, d->candidateRecordings, d->candidateAnnotations,
                                                        d->tierCorrespondances, d->tierNamesCommon, this);
    addPage(d->pageSelection);
    addPage(d->pageProcessMedia);
    addPage(d->pageAnalyse);
    addPage(d->pageCorrespondances);
    addPage(d->pageFinal);
}

ImportCorpusItemsWizard::~ImportCorpusItemsWizard()
{
    delete ui;
    delete d;
}
