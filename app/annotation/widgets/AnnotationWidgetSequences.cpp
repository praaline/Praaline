#include <QString>
#include <QList>
#include <QMap>
#include <QMessageBox>

#include "AnnotationWidgetSequences.h"
#include "ui_AnnotationWidgetSequences.h"

#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
#include "PraalineCore/Structure/AnnotationStructureLevel.h"
#include "PraalineCore/Structure/NameValueList.h"
using namespace Praaline::Core;

#include "pngui/model/annotation/AnnotationMultiTierTableModel.h"
#include "pngui/widgets/GridViewWidget.h"

struct AnnotationWidgetSequencesData {
    CorpusRepository *repository;
    AnnotationMultiTierTableModel *model;
    QMap<QString, NameValueList *> sequenceLevelsAndNVLs;
    QMap<QString, int> parentTierColumnIndexes;
    int columnIndexParentTier;
    QList<int> rowsSelected;

    AnnotationWidgetSequencesData() :
        repository(nullptr)
    {}
};

AnnotationWidgetSequences::AnnotationWidgetSequences(QWidget *parent) :
    QWidget(parent), ui(new Ui::AnnotationWidgetSequences), d(new AnnotationWidgetSequencesData())
{
    ui->setupUi(this);
    // When the user selects a sequence annotation level, show the appropriate labels from the name-value list
    connect(ui->comboBoxAnnotationLevel, SIGNAL(currentIndexChanged(int)), this, SLOT(annotationLevelChanged(int)));
    // Initial state: nothing selected
    ui->cmdAddSequence->setEnabled(false);
    // Make the sequences grid-view a little tighter
    ui->gridViewSequences->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridViewSequences->tableView()->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
}

AnnotationWidgetSequences::~AnnotationWidgetSequences()
{
    delete ui;
    delete d;
}


void AnnotationWidgetSequences::setCorpusRepositoryAndModel(Praaline::Core::CorpusRepository *repository, AnnotationMultiTierTableModel *model)
{
    // Clear previous data
    d->sequenceLevelsAndNVLs.clear();
    d->parentTierColumnIndexes.clear();
    ui->comboBoxAnnotationLevel->clear();
    ui->comboBoxSequenceLabel->clear();
    // Set corpus repository and model
    d->repository = repository;
    d->model = model;
    // Check we can proceed (nullptr in either of them is an indication that there is no data to annotate)
    if (!repository) return;
    if (!repository->annotationStructure()) return;
    if (!model) return;
    // Create the state from the repository and model
    auto displayedAttributes = d->model->displayedAttributes();
    foreach (AnnotationStructureLevel *level, d->repository->annotationStructure()->levels()) {
        if (level->levelType() == AnnotationStructureLevel::SequencesLevel) {
            // This level can be selected for sequence annotation
            ui->comboBoxAnnotationLevel->addItem(level->name(), level->ID());
            // This level has an associated name-value list
            NameValueList *nvl = d->repository->annotations()->getNameValueList(level->nameValueList());
            if (nvl) d->sequenceLevelsAndNVLs.insert(level->ID(), nvl);
            // Find the associated parent tier, used to display selected tokens. Find its column index in the model.
            for (int i = 0; i < displayedAttributes.count(); ++i) {
                if ((displayedAttributes.at(i).first == level->parentLevelID()) && (displayedAttributes.at(i).second.isEmpty())) {
                    d->parentTierColumnIndexes.insert(level->ID(), i + 3);
                }
            }

        }
    }
    annotationLevelChanged(0);
}

// private slot
void AnnotationWidgetSequences::annotationLevelChanged(int index)
{
    Q_UNUSED(index)
    // Clear
    ui->comboBoxSequenceLabel->clear();
    // Update everything to show the following sequence level ID (tier name):
    QString tiernameSequences = ui->comboBoxAnnotationLevel->currentData().toString();
    // Update the column index to the parent tier
    d->columnIndexParentTier = d->parentTierColumnIndexes.value(tiernameSequences, -1);
    // Update the available labels from the associated name-value list
    NameValueList *nvl = d->sequenceLevelsAndNVLs.value(tiernameSequences);
    if (nvl) {
        for (int i = 0; i < nvl->count(); ++i) {
            ui->comboBoxSequenceLabel->addItem(QString("%1\t%2").arg(nvl->displayString(i), nvl->value(i).toString()),
                                               nvl->value(i));
        }
    }
    // Update the sequences table model
    if (tiernameSequences.isEmpty()) {
        ui->gridViewSequences->tableView()->setModel(nullptr);
        return;
    }
    if (!d->model) return;
    if (!d->model->sequenceModel(tiernameSequences)) return;
    ui->gridViewSequences->tableView()->setModel(d->model->sequenceModel(tiernameSequences));
    ui->gridViewSequences->tableView()->resizeColumnsToContents();
}

void AnnotationWidgetSequences::setSelection(QList<int> rowsSelected)
{
    d->rowsSelected = rowsSelected;
    // Enable/Disable annotation
    if (d->rowsSelected.count() > 1) {
        ui->cmdAddSequence->setEnabled(true);
    }
    else {
        ui->cmdAddSequence->setEnabled(false);
    }
    // Display of the current selection
    QString currentSelection;
    for (int i = 0; i < d->rowsSelected.count(); i++) {
        QString tok = d->model->data(d->model->index(d->rowsSelected[i], d->columnIndexParentTier), Qt::EditRole).toString();
        currentSelection = currentSelection.append(tok).append(" ");
    }
    ui->labelCurrentSelection->setText(QString("Selection %1").arg(currentSelection));
}

// private slot
void AnnotationWidgetSequences::on_cmdAddSequence_clicked()
{
    if (d->rowsSelected.isEmpty()) return;
    QString tiernameSequences = ui->comboBoxAnnotationLevel->currentData().toString();
    QModelIndex indexFrom = d->model->index(d->rowsSelected.first(), d->columnIndexParentTier);
    QModelIndex indexTo   = d->model->index(d->rowsSelected.last(), d->columnIndexParentTier);
    bool ret = d->model->addSequence(tiernameSequences, indexFrom, indexTo, ui->comboBoxSequenceLabel->currentData().toString());
    if (!ret) {
        QMessageBox::warning(this, tr("Error"), tr("Error adding sequence. Ensure that the sequence begins and ends within utterances of the same speaker."),
                             QMessageBox::Ok);
    }
}

// private slot
void AnnotationWidgetSequences::on_cmdDeleteSequence_clicked()
{
    QModelIndexList selectedIndexes = ui->gridViewSequences->tableView()->selectionModel()->selectedIndexes();
    QList<int> rows;
    foreach (QModelIndex index, selectedIndexes)
        rows << index.row();
    QString tiernameSequences = ui->comboBoxAnnotationLevel->currentData().toString();
    if (!d->model->sequenceModel(tiernameSequences)) return;
    for (int i = rows.count() - 1; i >= 0; --i)
        d->model->sequenceModel(tiernameSequences)->removeSequence(rows.at(i));
}
