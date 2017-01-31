#include "ExportAnnotationsWizardPraatPage.h"
#include "ui_ExportAnnotationsWizardPraatPage.h"

#include <QStandardItemModel>
#include "pngui/model/CheckableProxyModel.h"
#include "pngui/model/corpus/AnnotationStructureTreeModel.h"

struct ExportAnnotationsWizardPraatPageData {
    ExportAnnotationsWizardPraatPageData () :
        modelAnnotations(0), modelLevelsAttributes(0), modelTextgridStructure(0)
    {}

    QStandardItemModel *modelAnnotations;
    AnnotationStructureTreeModel *modelLevelsAttributes;
    QStandardItemModel *modelTextgridStructure;
};

ExportAnnotationsWizardPraatPage::ExportAnnotationsWizardPraatPage(QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ExportAnnotationsWizardPraatPage)
{
    ui->setupUi(this);
    d = new ExportAnnotationsWizardPraatPageData();

//    d->modelLevelsAttributes = new AnnotationStructureTreeModel(corpus->annotationStructure(), true, true, this);
//    ui->treeviewLevelsAttributes->setModel(d->modelLevelsAttributes);

}

ExportAnnotationsWizardPraatPage::~ExportAnnotationsWizardPraatPage()
{
    delete ui;
}
