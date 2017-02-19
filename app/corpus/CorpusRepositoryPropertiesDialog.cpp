#include "pncore/datastore/CorpusRepository.h"
using namespace Praaline::Core;

#include "CorpusRepositoryPropertiesDialog.h"
#include "ui_CorpusRepositoryPropertiesDialog.h"

CorpusRepositoryPropertiesDialog::CorpusRepositoryPropertiesDialog(CorpusRepository *repository, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CorpusRepositoryPropertiesDialog)
{
    ui->setupUi(this);
}

CorpusRepositoryPropertiesDialog::~CorpusRepositoryPropertiesDialog()
{
    delete ui;
}
