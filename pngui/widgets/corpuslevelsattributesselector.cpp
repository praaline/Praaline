#include "corpuslevelsattributesselector.h"
#include "ui_corpuslevelsattributesselector.h"

CorpusLevelsAttributesSelector::CorpusLevelsAttributesSelector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CorpusLevelsAttributesSelector)
{
    ui->setupUi(this);
}

CorpusLevelsAttributesSelector::~CorpusLevelsAttributesSelector()
{
    delete ui;
}
