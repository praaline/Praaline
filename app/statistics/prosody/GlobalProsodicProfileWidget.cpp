#include <QDebug>
#include <QApplication>
#include <QProgressBar>
#include <QStandardItemModel>
#include <QFile>
#include <QFileInfo>

#include "GlobalProsodicProfileWidget.h"
#include "ui_GlobalProsodicProfileWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationDataTable.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginProsody {

struct GlobalProsodicProfileWidgetData {
    GlobalProsodicProfileWidgetData() :
        repository(0), gridviewResults(0), modelResults(0)
    {}

    CorpusRepository *repository;
    GridViewWidget *gridviewResults;
    QStandardItemModel *modelResults;
};

GlobalProsodicProfileWidget::GlobalProsodicProfileWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent), ui(new Ui::GlobalProsodicProfileWidget), d(new GlobalProsodicProfileWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;
    // Corpora combobox
    ui->comboBoxCorpus->addItems(repository->listCorporaIDs());
    // Command Analyse
    connect(ui->commandAnalyse, SIGNAL(clicked(bool)), this, SLOT(analyse()));
    // Results grid view
    d->gridviewResults = new GridViewWidget(this);
    d->gridviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutResults->addWidget(d->gridviewResults);
}

GlobalProsodicProfileWidget::~GlobalProsodicProfileWidget()
{
    delete ui;
    delete d;
}

void GlobalProsodicProfileWidget::analyse()
{
    if (!d->repository) return;
    QString corpusID = ui->comboBoxCorpus->currentText();
    QPointer<Corpus> corpus = d->repository->metadata()->getCorpus(corpusID);
    if (!corpus) return;

    QStandardItemModel *model = new QStandardItemModel(this);
    bool createHeaders = true;
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        if (!com) continue;
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            if (!rec) continue;
            foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
                if (!annot) continue;
                QStringList speakerIDs = d->repository->annotations()->getSpeakersInAnnotation(annot->ID());
                foreach (QString speakerID, speakerIDs) {
                    QFileInfo info(rec->filePath());
                    QString prosoPath = info.absoluteDir().absolutePath() + "/prosogram/";
                    QString filenameGlobalsheet = QString("%1_%2_globalsheet.txt").arg(rec->ID()).arg(speakerID);
                    AnnotationDataTable table;
                    if (!(table.readFromFile(prosoPath + filenameGlobalsheet))) {
                        qDebug() << "Error reading: " << filenameGlobalsheet;
                        continue;
                    }
                    // If this is the first table to process, create headers
                    if (createHeaders) {
                        QStringList headers;
                        headers << "CommunicationID" << "RecordingID" << "SpeakerID";
                        headers << table.getFieldNames();
                        model->setHorizontalHeaderLabels(headers);
                        createHeaders = false;
                    }
                    // Process data from table
                    for (int i = 0; i < table.getRowCount(); i++) {
                        QList<QStandardItem *> items;
                        items.append(new QStandardItem(com->ID()));
                        items.append(new QStandardItem(rec->ID()));
                        items.append(new QStandardItem(speakerID));
                        foreach (QString attributeName, table.getFieldNames()) {
                            QVariant value = table.getData(i, attributeName);
                            QStandardItem *item = new QStandardItem();
                            item->setData(value, Qt::DisplayRole);
                            items.append(item);
                        }
                        model->appendRow(items);
                    }
                } // speaker
            } // annotation
        } // recording
    } // commmunication

    // Update table
    d->gridviewResults->tableView()->setModel(model);
    if (d->modelResults) { d->modelResults->clear(); delete d->modelResults; }
    d->modelResults = model;
}

} // namespace StatisticsPluginProsody
} // namespace Plugins
} // namespace Praaline
