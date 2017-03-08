#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/CorpusRepositoryDefinition.h"
using namespace Praaline::Core;

#include "CorpusRepositoryPropertiesDialog.h"
#include "ui_CorpusRepositoryPropertiesDialog.h"

struct CorpusRepositoryPropertiesDialogData {
    CorpusRepositoryDefinition repositoryDef;
};

CorpusRepositoryPropertiesDialog::CorpusRepositoryPropertiesDialog(const CorpusRepositoryDefinition &repositoryDef, QWidget *parent) :
    QDialog(parent), ui(new Ui::CorpusRepositoryPropertiesDialog), d(new CorpusRepositoryPropertiesDialogData)
{
    ui->setupUi(this);
    d->repositoryDef = repositoryDef;
    updateFields();
}

CorpusRepositoryPropertiesDialog::~CorpusRepositoryPropertiesDialog()
{
    delete ui;
    delete d;
}

void CorpusRepositoryPropertiesDialog::updateFields()
{
    ui->labelRepositoryDefinitionFilename->setText(d->repositoryDef.filenameDefinition);
    ui->editCorpusRepositoryID->setText(d->repositoryDef.repositoryID);
    ui->editCorpusRepositoryDescription->setText(d->repositoryDef.repositoryName);
    ui->editBasePath->setText(d->repositoryDef.basePath);
    ui->editBasePathMedia->setText(d->repositoryDef.basePathMedia);
}

void CorpusRepositoryPropertiesDialog::accept()
{
    d->repositoryDef.repositoryID = ui->editCorpusRepositoryID->text();
    d->repositoryDef.repositoryName = ui->editCorpusRepositoryDescription->text();
    d->repositoryDef.basePath = ui->editBasePath->text();
    d->repositoryDef.basePathMedia = ui->editBasePathMedia->text();
    QDialog::accept();
}

CorpusRepositoryDefinition &CorpusRepositoryPropertiesDialog::repositoryDef() const
{
    return d->repositoryDef;
}
