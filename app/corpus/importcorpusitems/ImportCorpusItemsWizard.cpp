#include <QSet>
#include <QMap>
#include <QMultiHash>
#include "ImportCorpusItemsWizardSelectionPage.h"
#include "ImportCorpusItemsWizardProcessMediaPage.h"
#include "ImportCorpusItemsWizardAnalysePage.h"
#include "ImportCorpusItemsWizardCorrespondancesPage.h"
#include "ImportCorpusItemsWizardFinalPage.h"

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Interfaces/ImportAnnotations.h"
using namespace Praaline::Core;

#include "ImportCorpusItemsWizard.h"
#include "ui_ImportCorpusItemsWizard.h"

struct ImportCorpusItemsWizardData {
    ImportCorpusItemsWizardData() :
        pageSelection(nullptr), pageProcessMedia(nullptr), pageAnalyse(nullptr),
        pageCorrespondances(nullptr), pageFinal(nullptr),
        repository(nullptr)
    {}

    // Pages
    ImportCorpusItemsWizardSelectionPage *pageSelection;
    ImportCorpusItemsWizardProcessMediaPage *pageProcessMedia;
    ImportCorpusItemsWizardAnalysePage *pageAnalyse;
    ImportCorpusItemsWizardCorrespondancesPage *pageCorrespondances;
    ImportCorpusItemsWizardFinalPage *pageFinal;

    // State
    CorpusRepository *repository;
    QMap<QPair<QString, QString>, CorpusRecording *> candidateRecordings;
    QMap<QPair<QString, QString>, CorpusAnnotation *> candidateAnnotations;
    QMultiHash<QString, TierCorrespondance> tierCorrespondances;
    QSet<QString> tierNamesCommon;
};

ImportCorpusItemsWizard::ImportCorpusItemsWizard(CorpusRepository *repository, QWidget *parent) :
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
