#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QPair>
#include <QVariant>
#include <QDir>
#include <QFileInfo>
#include <QStandardItemModel>
#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/annotation/Interval.h"

#include "pnlib/mediautil/AudioSegmenter.h"

#include "SplitCommunicationsDialog.h"
#include "ui_SplitCommunicationsDialog.h"


struct SplitCommunicationsDialogData{
    SplitCommunicationsDialogData() : corpus(0), modelCorpusCommunications(0), modelDistinctValues(0) {}

    QPointer<Corpus> corpus;
    QStandardItemModel *modelCorpusCommunications;
    QStandardItemModel *modelDistinctValues;
};

SplitCommunicationsDialog::SplitCommunicationsDialog(QPointer<Corpus> corpus, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SplitCommunicationsDialog), d(new SplitCommunicationsDialogData)
{
    ui->setupUi(this);
    d->corpus = corpus;

    connect(ui->commandClose, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->comboBoxLevel, SIGNAL(currentIndexChanged(QString)), this, SLOT(annotationLevelChanged()));
    connect(ui->comboBoxAttribute, SIGNAL(currentIndexChanged(QString)), this, SLOT(annotationAttributeChanged()));
    connect(ui->commandSplit, SIGNAL(clicked()), this, SLOT(doSplit()));

    if (!corpus) return;
    d->modelCorpusCommunications = new QStandardItemModel(this);
    int row = 0;
    foreach (QString communicationID, corpus->communicationIDs()) {
        QStandardItem *item = new QStandardItem(communicationID);
        item->setCheckable(true); item->setCheckState(Qt::Checked);
        d->modelCorpusCommunications->setItem(row, 0, item);
        row++;
    }
    d->modelCorpusCommunications->setHorizontalHeaderLabels(QStringList() << tr("Communication ID"));
    ui->treeViewCorpusItems->setModel(d->modelCorpusCommunications);

    ui->comboBoxLevel->addItems(d->corpus->repository()->annotationStructure()->levelIDs());
    annotationLevelChanged();
}

SplitCommunicationsDialog::~SplitCommunicationsDialog()
{
    delete ui;
    delete d;
}

void SplitCommunicationsDialog::annotationLevelChanged()
{
    if (!d->corpus) return;
    ui->comboBoxAttribute->clear();
    QString levelID = ui->comboBoxLevel->currentText();
    if (!d->corpus->repository()->annotationStructure()->hasLevel(levelID)) return;
    ui->comboBoxAttribute->addItem("");
    ui->comboBoxAttribute->addItems(d->corpus->repository()->annotationStructure()->level(levelID)->attributeIDs());
    ui->comboBoxAttribute->setCurrentText("");
    annotationAttributeChanged();
}

void SplitCommunicationsDialog::annotationAttributeChanged()
{
    if (!d->corpus) return;
    QString levelID = ui->comboBoxLevel->currentText();
    QString attributeID = ui->comboBoxAttribute->currentText();
    QStringList attributeIDs; attributeIDs << attributeID;
    QList<QPair<QList<QVariant>, long> > tuples = d->corpus->repository()->annotations()->getDistinctLabels(levelID, attributeIDs);
    if (d->modelDistinctValues) delete d->modelDistinctValues;
    d->modelDistinctValues = new QStandardItemModel(this);
    for (int row = 0; row < tuples.count(); ++row) {
        QList<QVariant> attributeValues = tuples[row].first;
        for (int col = 0; col < attributeValues.count(); ++col) {
            QStandardItem *item = new QStandardItem(attributeValues[col].toString());
            if (col == 0) item->setCheckable(true); item->setCheckState(Qt::Checked);
            d->modelDistinctValues->setItem(row, col, item);
        }
        d->modelDistinctValues->setItem(row, attributeValues.count(), new QStandardItem(QString::number(tuples[row].second)));
    }
    d->modelDistinctValues->setHorizontalHeaderLabels(QStringList() << tr("Value") << tr("No of occurrences"));
    ui->treeViewDistinctValues->setModel(d->modelDistinctValues);
}


void SplitCommunicationsDialog::doSplit()
{
    if (!d->corpus) return;
    if (!d->modelCorpusCommunications) return;
    if (!d->modelDistinctValues) return;
    QString levelID = ui->comboBoxLevel->currentText();
    if (!d->corpus->repository()->annotationStructure()->hasLevel(levelID)) return;
    QString attributeID = ui->comboBoxAttribute->currentText();
    //
    QStringList selectedCommunicationIDs;
    for (int row = 0; row < d->modelCorpusCommunications->rowCount(); ++row) {
        QStandardItem *item = d->modelCorpusCommunications->item(row, 0);
        if (item->checkState() == Qt::Checked) selectedCommunicationIDs << item->text();
    }
    QStringList selectedSplitLabels;
    for (int row = 0; row < d->modelDistinctValues->rowCount(); ++row) {
        QStandardItem *item = d->modelDistinctValues->item(row, 0);
        if (item->checkState() == Qt::Checked) selectedSplitLabels << item->text();
    }
    if (selectedCommunicationIDs.isEmpty() || selectedSplitLabels.isEmpty()) return;
    //
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(selectedCommunicationIDs.count());
    int progress = 0;
    foreach (QString communicationID, d->corpus->communicationIDs()) {
        if (!selectedCommunicationIDs.contains(communicationID)) continue;
        QPointer<CorpusCommunication> com = d->corpus->communication(communicationID);
        if (!com) continue;
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            if (!rec) continue;
            QString originalFilename = rec->filePath();
            QString originalPath = QFileInfo(originalFilename).canonicalPath();
            QDir dirMedia = QDir(rec->basePath());
            foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
                if (!annot) continue;
                QList<Interval *> segments;
                int i = 1;
                QList<Interval *> intervals = d->corpus->repository()->annotations()->getIntervals(
                            AnnotationDatastore::Selection(annot->ID(), "", levelID));
                foreach (Interval *intv, intervals) {
                    QString label = (attributeID.isEmpty()) ? intv->text() : intv->attribute(attributeID).toString();
                    if (selectedSplitLabels.contains(label)) {
                        Interval *segment = new Interval(intv);
                        segment->setAttribute("splitCommunicationID", QString("%1_%2_%3")
                                              .arg(rec->ID()).arg(i, 2, 10, QChar('0')).arg(label));
                        segments << segment;
                        i++;
                    }
                }
                AudioSegmenter::segment(originalFilename, originalPath, segments, "splitCommunicationID");
                // Add communications
                foreach (Interval *segment, segments) {
                    QString ID = segment->attribute("splitCommunicationID").toString();
                    CorpusCommunication *segmentCom = new CorpusCommunication(ID);
                    CorpusRecording *segmentRec = new CorpusRecording(ID);
                    segmentRec->setName(ID);
                    segmentRec->setFilename(dirMedia.relativeFilePath(originalPath + "/" + ID + ".wav"));
                    segmentCom->addRecording(segmentRec);
                    d->corpus->addCommunication(segmentCom);
                }
                // 
                qDeleteAll(segments);
                segments.clear();
            }
        }
        progress++;
        ui->progressBar->setValue(progress);
        QApplication::processEvents();
    }
}

