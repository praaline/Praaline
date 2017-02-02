#include <QString>
#include <QHash>
#include <QStandardItemModel>
#include "ExportMetadataWizard.h"
#include "ui_ExportMetadataWizard.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/structure/MetadataStructure.h"
#include "CorpusRepositoriesManager.h"
using namespace Praaline::Core;

struct ExportMetadataWizardData {
    ExportMetadataWizardData() {}

    QPointer<CorpusRepositoriesManager> corpusRepositoriesManager;
    QString repositoryID;
    QList<CorpusObject::Type> corpusObjectTypes;
    QSharedPointer<QStandardItemModel> modelCorpora;
    QSharedPointer<QStandardItemModel> modelCorpusObjectTypes;
    QHash<CorpusObject::Type, QSharedPointer<QStandardItemModel> > modelsAttributes;
};

ExportMetadataWizard::ExportMetadataWizard(QWidget *parent) :
    QWizard(parent), ui(new Ui::ExportMetadataWizard), d(new ExportMetadataWizardData)
{
    ui->setupUi(this);

    // Supported corpus object types having exportable metadata
    d->corpusObjectTypes << CorpusObject::Type_Corpus << CorpusObject::Type_Communication << CorpusObject::Type_Speaker <<
                            CorpusObject::Type_Recording << CorpusObject::Type_Annotation << CorpusObject::Type_Participation;
    // Corpus repositories
    foreach (QObject* obj, OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager")) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    if (!d->corpusRepositoriesManager) this->reject();
    // Repository combobox
    ui->comboBoxRepository->addItems(d->corpusRepositoriesManager->listCorpusRepositoryIDs());
    connect(ui->comboBoxRepository, SIGNAL(currentTextChanged(QString)), this, SLOT(corpusRepositoryChanged(QString)));
    corpusRepositoryChanged(d->corpusRepositoriesManager->activeCorpusRepositoryID());
    // Export formats
    ui->comboBoxExportFormat->addItems(QStringList() << tr("Tab-separated Text Files (*.txt)"));
    ui->comboBoxExportFormat->setCurrentIndex(0);
    connect(ui->comboBoxExportFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(exportFormatChanged(int)));
    // Select file or folder button

    // List of corpus objects
    d->modelCorpusObjectTypes = QSharedPointer<QStandardItemModel>(new  QStandardItemModel(this));
    d->modelCorpusObjectTypes->setColumnCount(1);
    int i = 0;
    QStringList corpusObjectTypeNames;
    corpusObjectTypeNames << tr("Corpus") << tr("Communication") << tr("Speaker") << tr("Recording") << tr("Annotation") << tr("Participation");
    foreach (CorpusObject::Type type, d->corpusObjectTypes) {
        QStandardItem *item = new QStandardItem(corpusObjectTypeNames.at(i));
        item->setData(type);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        d->modelCorpusObjectTypes->setItem(i, item);
        ++i;
    }
    ui->treeViewCorpusObjectTypes->setModel(d->modelCorpusObjectTypes.data());
    // Presentation
    ui->treeViewRepositoriesCorpora->setHeaderHidden(true);
    ui->treeViewCorpusObjectTypes->setHeaderHidden(true);
    ui->treeViewMetadataAttributes->setHeaderHidden(true);
}

ExportMetadataWizard::~ExportMetadataWizard()
{
    delete ui;
    delete d;
}

QStandardItemModel *ExportMetadataWizard::createAttributeModel(CorpusObject::Type type, bool checkable)
{
    if (d->repositoryID.isEmpty()) return Q_NULLPTR;
    if (!d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID)) return Q_NULLPTR;
    if (!d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID)->metadataStructure()) return Q_NULLPTR;
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(1);
    int i = 0;
    foreach (QPointer<MetadataStructureAttribute> attribute,
             d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID)->metadataStructure()->attributes(type)) {
        if (!attribute) continue;
        QStandardItem *item = new QStandardItem(attribute->name());
        item->setData(attribute->ID());
        item->setCheckable(checkable);
        if (checkable) item->setCheckState(Qt::Checked);
        model->setItem(i, item);
        ++i;
    }
    return model;
}


void ExportMetadataWizard::exportFormatChanged(int)
{

}

void ExportMetadataWizard::corpusRepositoryChanged(QString repositoryID)
{
    if (d->repositoryID == repositoryID) return;
    if (!d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID)) return;
    d->repositoryID = repositoryID;
    // Update the list of corpora
    d->modelCorpora = QSharedPointer<QStandardItemModel>(new QStandardItemModel(this));
    d->modelCorpora->setColumnCount(1);
    int i = 0;
    foreach (QString corpusID, d->corpusRepositoriesManager->listAvailableCorpusIDs(repositoryID)) {
        QStandardItem *item = new QStandardItem(corpusID);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        d->modelCorpora->setItem(i, item);
        ++i;
    }
    ui->treeViewRepositoriesCorpora->setModel(d->modelCorpora.data());
    // Update the metadata attribute models for each corpus object type
    foreach (CorpusObject::Type type, d->corpusObjectTypes) {
        d->modelsAttributes.insert(type, QSharedPointer<QStandardItemModel>(createAttributeModel(type, true)));
    }
    ui->treeViewMetadataAttributes->setModel(d->modelsAttributes.value(CorpusObject::Type_Corpus));
}
