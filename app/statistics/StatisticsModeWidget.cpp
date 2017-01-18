#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QHash>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QStandardItemModel>
#include <QDebug>
#include "StatisticsModeWidget.h"
#include "ui_StatisticsModeWidget.h"

#include "InterraterAgreement.h"
#include "AnalyserTemporal.h"
#include "AnalyserPitch.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/AnnotationTier.h"
#include "pncore/structure/MetadataStructure.h"
using namespace Praaline::Core;

#include "pngui/observers/CorpusObserver.h"
#include "pngui/widgets/GridViewWidget.h"
#include "CorpusRepositoriesManager.h"

#include <QtilitiesExtensionSystem>
using namespace QtilitiesExtensionSystem;

struct StatisticsModeWidgetData {
    StatisticsModeWidgetData() :
        actionAnnotate(0), actionSaveOutput(0), actionClearOutput(0),
        textResults(0), runningPlugins(0), corpusRepositoriesManager(0), tableResults(0), modelResults(0)
    {}

    QAction *actionAnnotate;
    QAction *actionSaveOutput;
    QAction *actionClearOutput;

    QTextEdit *textResults;
    int runningPlugins;

    CorpusRepositoriesManager *corpusRepositoriesManager;
    GridViewWidget *tableResults;
    QStandardItemModel *modelResults;
};


StatisticsModeWidget::StatisticsModeWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::StatisticsModeWidget), d(new StatisticsModeWidgetData)
{
    ui->setupUi(this);
    // setupActions();

    // Corpora manager
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
//    connect(d->corporaManager, SIGNAL(activeCorpusChanged(QString)), this, SLOT(activeCorpusChanged(QString)));

    // Output messages of running statistical analysis plugins
    d->textResults = new QTextEdit(this);
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    d->textResults->setFont(fixedFont);
    ui->gridLayoutMessages->addWidget(d->textResults);

    // Command Analyse
    connect(ui->commandAnalyse, SIGNAL(clicked(bool)), this, SLOT(analyse()));

    // Results grid
    d->tableResults = new GridViewWidget(this);
    d->tableResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutMessages->addWidget(d->tableResults);

    // Splitter
    ui->splitterLeftRight->setSizes(QList<int>() << 50 << 300);
}

StatisticsModeWidget::~StatisticsModeWidget()
{
    delete ui;
    delete d;
}

void StatisticsModeWidget::actionSaveOutput()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Automatic Annotation Output"),
                                "praaline_report.txt", tr("Text File (*.txt);;All Files (*)"),
                                &selectedFilter, options);
    if (fileName.isEmpty()) return;
    QFile fileOut(fileName);
    if (! fileOut.open(QFile::WriteOnly | QFile::Text)) return;
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    out << d->textResults->document()->toPlainText();
    fileOut.close();
}

void StatisticsModeWidget::actionClearOutput()
{
    d->textResults->clear();
}

void StatisticsModeWidget::analyseFromFile()
{
//    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
//    if (!corpus) return;
//    QScopedPointer<AnalyserPitch> analyser(new AnalyserPitch);

//    d->textResults->setWordWrapMode(QTextOption::NoWrap);

//    QStandardItemModel *model = new QStandardItemModel(this);

//    QFile file("/home/george/subintervals.txt");
//    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return;
//    QTextStream stream(&file);
//    QList<Interval *> intervals;
//    QString currentCommunicationID, currentAnnotationID;
//    do {
//        QString line = stream.readLine().trimmed();
//        // CommunicationID	AnnotationID	Occurrence	Part	tMin	tMax
//        QString communicationID = line.section("\t", 0, 0);
//        QString annotationID = line.section("\t", 1, 1);
//        QString label = line.section("\t", 2, 3);
//        RealTime tMin = RealTime::fromSeconds(line.section("\t", 4, 4).toDouble());
//        RealTime tMax = RealTime::fromSeconds(line.section("\t", 5, 5).toDouble());

//        annotationID.replace("G01B_DRIV_S2_4035_4516", "01_CelineB_G01B_DRIV_S2_4035_4516");
//        annotationID.replace("G01B_PASS_S1_4035_4516", "02_Gervaise_G01B_PASS_S1_4035_4516");
//        annotationID.replace("G02A_DRIV_S1_3156_3954", "03_AlineC_G02A_DRIV_S1_3156_3954");
//        annotationID.replace("G02A_PASS_S2_3156_3954", "04_AdeleF_G02A_PASS_S2_3156_3954");
//        annotationID.replace("G02B_DRIV_S2_3535_4239", "05_MorganeL_G02B_DRIV_S2_3535_4239");
//        annotationID.replace("G02B_PASS_S1_3532_4239", "06_CamilleS_G02B_PASS_S1_3532_4239");
//        annotationID.replace("G03B_DRIV_S2_2620_3138", "07_AdelaideC_G03B_DRIV_S2_2620_3138");
//        annotationID.replace("G03B_PASS_S1_2620_3138", "08_EmilieDuv_G03B_PASS_S1_2620_3138");
//        annotationID.replace("G04B_DRIV_S2_5626_6229", "09_AntoinetteT_G04B_DRIV_S2_5626_6229");
//        annotationID.replace("G04B_PASS_S1_5626_6229", "10_AliceP_G04B_PASS_S1_5626_6229");
//        annotationID.replace("G05B_DRIV_S2_2358_2820", "11_EmilieLaf_G05B_DRIV_S2_2358_2820");
//        annotationID.replace("G05B_PASS_S1_2358_2820", "12_DeysiY_G05B_PASS_S1_2358_2820");

//        if (currentAnnotationID == annotationID) {
//            intervals << new Interval(tMin, tMax, label);
//        }
//        else {
//            if (!intervals.isEmpty()) {
//                analyser->calculate(corpus, currentCommunicationID, currentAnnotationID, intervals);
//                for (int i = 0; i < analyser->model()->rowCount(); ++i) {
//                    model->appendRow(analyser->model()->takeRow(i));
//                }
//                qDeleteAll(intervals);
//                intervals.clear();
//            }
//            currentCommunicationID = communicationID;
//            currentAnnotationID = annotationID;
//            intervals << new Interval(tMin, tMax, label);
//        }
//    } while (!stream.atEnd());
//    file.close();
//    if (!intervals.isEmpty()) {
//        analyser->calculate(corpus, currentCommunicationID, currentAnnotationID, intervals);
//        for (int i = 0; i < analyser->model()->rowCount(); ++i) {
//            model->appendRow(analyser->model()->takeRow(i));
//        }
//        qDeleteAll(intervals);
//        intervals.clear();
//    }
//    // Update table headers
//    if (analyser && analyser->model()) {
//        for (int i = 0; i < analyser->model()->columnCount(); ++i)
//            model->setHorizontalHeaderItem(i, analyser->model()->horizontalHeaderItem(i));
//    }
//    // Update table
//    d->tableResults->tableView()->setModel(model);
//    if (d->modelResults) { d->modelResults->clear(); delete d->modelResults; }
//    d->modelResults = model;
}

//for (int i = 0; i < analyser->model()->rowCount(); ++i) {
//    QString line;
//    for (int j = 0; j < analyser->model()->columnCount(); ++j) {
//        line.append(analyser->model()->item(i, j)->data(Qt::DisplayRole).toString()).append("\t");
//    }
//    if (!line.isEmpty()) line.chop(1);
//    d->textResults->append(line);
//}

void StatisticsModeWidget::analyseT()
{
//    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
//    if (!corpus) return;
//    QScopedPointer<AnalyserTemporal>analyser(new AnalyserTemporal);
//    // Pause lists
//    QFile filePauseListSIL(corpus->basePath() + "/pauselist_SIL.txt");
//    if ( !filePauseListSIL.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
//    QTextStream pauseListSIL(&filePauseListSIL);
//    pauseListSIL.setCodec("UTF-8");
//    QFile filePauseListFIL(corpus->basePath() + "/pauselist_FIL.txt");
//    if ( !filePauseListFIL.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
//    QTextStream pauseListFIL(&filePauseListFIL);
//    pauseListFIL.setCodec("UTF-8");

//    // Models available
//    QStandardItemModel *modelCom = new QStandardItemModel(this);
//    QStandardItemModel *modelSpk = new QStandardItemModel(this);
//    // Create model headers
//    QStringList labels;
//    labels << "CommunicationID";
//    foreach (QPointer<MetadataStructureAttribute> attr, corpus->metadataStructure()->attributes(CorpusObject::Type_Communication))
//        labels << attr->ID();
//    foreach (QString measureID, analyser->measureIDsForCommunication()) labels << measureID;
//    modelCom->setHorizontalHeaderLabels(labels);
//    labels.clear();
//    labels << "CommunicationID" << "SpeakerID";
//    pauseListSIL << "CommunicationID\tSpeakerID\t";
//    pauseListFIL << "CommunicationID\tSpeakerID\t";
//    foreach (QPointer<MetadataStructureAttribute> attr, corpus->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
//        labels << attr->ID();
//        pauseListSIL << attr->ID() << "\t";
//        pauseListFIL << attr->ID() << "\t";
//    }
//    foreach (QPointer<MetadataStructureAttribute> attr, corpus->metadataStructure()->attributes(CorpusObject::Type_Speaker)) {
//        labels << attr->ID();
//    }
//    foreach (QString measureID, analyser->measureIDsForSpeaker()) labels << measureID;
//    modelSpk->setHorizontalHeaderLabels(labels);
//    pauseListSIL << "Duration\tDurationRel1\tDurationRel2\tDurationRel3\tDurationRel4\tDurationRel5\n";
//    pauseListFIL << "Duration\n";
//    // Analyse communications / and then speakers
//    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
//        analyser->calculate(corpus, com, pauseListSIL, pauseListFIL);
//        QList<QStandardItem *> itemsCom;
//        QStandardItem *item;
//        item = new QStandardItem(); item->setData(com->ID(), Qt::DisplayRole); itemsCom << item;
//        // properties
//        foreach (QPointer<MetadataStructureAttribute> attr, corpus->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
//            item = new QStandardItem(); item->setData(com->property(attr->ID()), Qt::DisplayRole); itemsCom << item;
//        }
//        // measures
//        foreach (QString measureID, analyser->measureIDsForCommunication()) {
//            // analyser->measureDefinitionForCommunication(measureID).displayNameUnit()
//            item = new QStandardItem(); item->setData(analyser->measureCom(measureID), Qt::DisplayRole); itemsCom << item;
//        }
//        modelCom->appendRow(itemsCom);

//        foreach (QString speakerID, analyser->speakerIDs()) {
//            QList<QStandardItem *> itemsSpk;
//            item = new QStandardItem(); item->setData(com->ID(), Qt::DisplayRole); itemsSpk << item;
//            item = new QStandardItem(); item->setData(speakerID, Qt::DisplayRole); itemsSpk << item;
//            // properties
//            foreach (QPointer<MetadataStructureAttribute> attr, corpus->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
//                item = new QStandardItem(); item->setData(com->property(attr->ID()), Qt::DisplayRole); itemsSpk << item;
//            }
//            foreach (QPointer<MetadataStructureAttribute> attr, corpus->metadataStructure()->attributes(CorpusObject::Type_Speaker)) {
//                item = new QStandardItem(); item->setData(com->property(attr->ID()), Qt::DisplayRole); itemsSpk << item;
//            }
//            // measures
//            foreach (QString measureID, analyser->measureIDsForSpeaker()) {
//                // analyser->measureDefinitionForSpeaker(measureID).displayNameUnit();
//                item = new QStandardItem(); item->setData(analyser->measureSpk(speakerID, measureID), Qt::DisplayRole); itemsSpk << item;
//            }
//            modelSpk->appendRow(itemsSpk);
//        }

//    }

//    // Update table headers
////    for (int i = 0; i < modelCom->columnCount(); ++i)
////        model->setHorizontalHeaderItem(i, new QStandardItem(analyser->model()->horizontalHeaderItem(i)->text()));
//    // Update table
//    d->tableResults->tableView()->setModel(modelSpk);
//    if (d->modelResults) { d->modelResults->clear(); delete d->modelResults; }
//    d->modelResults = modelSpk;

//    filePauseListSIL.close();
//    filePauseListFIL.close();
}

void StatisticsModeWidget::analyse()
{
    analyseT();
    return;
//    analyseFromFile();
//    return;

//    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
//    if (!corpus) return;
//    QScopedPointer<AnalyserPitch> analyser(new AnalyserPitch);

//    QStandardItemModel *model = new QStandardItemModel(this);
//    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
//        if (!com) continue;
//        foreach (QString annotationID, com->annotationIDs()) {
//            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()
//                    ->getTiersAllSpeakers(annotationID, QStringList() << "transcription");
//            foreach (QString speakerID, tiersAll.keys()) {
//                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
//                if (!tiers) continue;
//                IntervalTier *tier_macroUnit = tiers->getIntervalTierByName("transcription");
//                QList<Interval *> macroUnitIntervals;
//                foreach (Interval *intv, tier_macroUnit->intervals()) {
//                    if (!intv->isPauseSilent()) macroUnitIntervals << intv;
//                }
//                // Run analyser for each macro-unit (excluding pauses) and take each row of results into the model
//                analyser->calculate(corpus, com->ID(), annotationID, macroUnitIntervals);
//                for (int i = 0; i < analyser->model()->rowCount(); ++i) {
//                    model->appendRow(analyser->model()->takeRow(i));
//                }
//            }
//            qDeleteAll(tiersAll);
//        }
//    }
//    // Update table headers
//    if (analyser && analyser->model()) {
//        for (int i = 0; i < analyser->model()->columnCount(); ++i)
//            model->setHorizontalHeaderItem(i, new QStandardItem(analyser->model()->horizontalHeaderItem(i)->text()));
//    }
//    // Update table
//    d->tableResults->tableView()->setModel(model);
//    if (d->modelResults) { d->modelResults->clear(); delete d->modelResults; }
//    d->modelResults = model;

}

