#include "CreateSequenceAnnotationDialog.h"
#include "ui_CreateSequenceAnnotationDialog.h"

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "annotation/IntervalTier.h"
#include "annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

#include "CorpusRepositoriesManager.h"

struct CreateSequenceAnnotationDialogData {
    CreateSequenceAnnotationDialogData() :
        corpusRepositoriesManager(0)
    {}

    QPointer<CorpusRepositoriesManager> corpusRepositoriesManager;
    QString repositoryID;
};


CreateSequenceAnnotationDialog::CreateSequenceAnnotationDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::CreateSequenceAnnotationDialog), d(new CreateSequenceAnnotationDialogData)
{
    ui->setupUi(this);
}

CreateSequenceAnnotationDialog::~CreateSequenceAnnotationDialog()
{
    delete d;
    delete ui;
}
