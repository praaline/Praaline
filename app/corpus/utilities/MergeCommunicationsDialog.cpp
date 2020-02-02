#include "MergeCommunicationsDialog.h"
#include "ui_MergeCommunicationsDialog.h"

#include "PraalineCore/Corpus/Corpus.h"
using namespace Praaline::Core;

MergeCommunicationsDialog::MergeCommunicationsDialog(Corpus *corpus, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MergeCommunicationsDialog)
{
    ui->setupUi(this);
}

MergeCommunicationsDialog::~MergeCommunicationsDialog()
{
    delete ui;
}
