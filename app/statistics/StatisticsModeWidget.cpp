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
#include "ui_statisticsmodewidget.h"

#include "InterraterAgreement.h"
#include "AnalyserTemporal.h"

#include "pncore/corpus/corpus.h"
#include "pncore/annotation/annotationtier.h"
#include "pngui/observers/corpusobserver.h"
#include "pngui/widgets/gridviewwidget.h"
#include "CorporaManager.h"

#include <QtilitiesExtensionSystem>
using namespace QtilitiesExtensionSystem;

struct StatisticsModeWidgetData {
    StatisticsModeWidgetData() :
        actionAnnotate(0), actionSaveOutput(0), actionClearOutput(0),
        textResults(0), runningPlugins(0), corporaManager(0), tableResults(0), modelResults(0)
    {}

    QAction *actionAnnotate;
    QAction *actionSaveOutput;
    QAction *actionClearOutput;

    QTextEdit *textResults;
    int runningPlugins;

    CorporaManager *corporaManager;
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
    list = OBJECT_MANAGER->registeredInterfaces("CorporaManager");
    foreach (QObject* obj, list) {
        CorporaManager *manager = qobject_cast<CorporaManager *>(obj);
        if (manager) d->corporaManager = manager;
    }
//    connect(d->corporaManager, SIGNAL(activeCorpusChanged(QString)), this, SLOT(activeCorpusChanged(QString)));

    // Output messages of running statistical analysis plugins
    d->textResults = new QTextEdit(this);
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    d->textResults->setFont(fixedFont);
    ui->gridLayoutMessages->addWidget(d->textResults);

    connect(ui->commandAnalyse, SIGNAL(clicked(bool)), this, SLOT(analyse()));

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

void StatisticsModeWidget::analyse()
{
    AnalyserTemporal *analyser = new AnalyserTemporal(this);

    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        analyser->calculate(corpus, com);
        QString line;
        line.append("\t").append(com->ID());
        d->textResults->append(line);
        foreach (QString measureID, analyser->measureIDsForCommunication()) {
            line = analyser->measureDefinitionForCommunication(measureID).displayNameUnit();
            line.append(QString("\t%1").arg(analyser->measureCom(measureID)));
            d->textResults->append(line);
        }
        line.append("\n");
        d->textResults->append(line);
        //
        line.clear();
        foreach (QString speakerID, analyser->speakerIDs()) {
            line.append("\t").append(speakerID);
        }
        line.append("\n");
        d->textResults->append(line);
        foreach (QString measureID, analyser->measureIDsForSpeaker()) {
            line = analyser->measureDefinitionForSpeaker(measureID).displayNameUnit();
            foreach (QString speakerID, analyser->speakerIDs()) {
                line.append(QString("\t%1").arg(analyser->measureSpk(speakerID, measureID)));
            }
            d->textResults->append(line);
        }
        line.append("\n");
        d->textResults->append(line);
    }

    delete analyser;
}
