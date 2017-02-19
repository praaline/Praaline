#include <QGridLayout>
#include <QTreeView>

#include "CorpusLevelsAttributesSelector.h"
#include "ui_corpuslevelsattributesselector.h"

struct CorpusLevelsAttributesSelectorData {
    QGridLayout *gridLayout;
    QTreeView *treeviewSelection;
};

CorpusLevelsAttributesSelector::CorpusLevelsAttributesSelector(QWidget *parent) :
    QWidget(parent), d(new CorpusLevelsAttributesSelectorData)
{
    d->gridLayout = new QGridLayout(this);
    d->gridLayout->setObjectName(QStringLiteral("gridLayout"));
    d->gridLayout->setContentsMargins(0, 0, 0, 0);
    d->treeviewSelection = new QTreeView(this);
    d->treeviewSelection->setObjectName(QStringLiteral("treeviewSelection"));

    d->gridLayout->addWidget(d->treeviewSelection, 0, 0, 1, 1);
}

CorpusLevelsAttributesSelector::~CorpusLevelsAttributesSelector()
{
    delete d;
}
