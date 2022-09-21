#include <QString>
#include <QWidget>
#include <QList>
#include <QMap>
#include <QLayoutItem>

#include <QtCharts/QChartView>
#include <QtCharts/QBoxPlotSeries>
#include <QtCharts/QBoxSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
QT_CHARTS_USE_NAMESPACE

#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Structure/MetadataStructure.h"
#include "PraalineCore/Statistics/StatisticalSummary.h"
using namespace Praaline::Core;

#include "StatisticalAnalyserBase.h"

#include "StatisticalAnalysisChartsWidget.h"
#include "ui_StatisticalAnalysisChartsWidget.h"


struct StatisticalAnalysisChartsWidgetData {
    StatisticalAnalyserBase *analyser;

    CorpusRepository *repository;
    bool showMeasuresForCom;

    StatisticalAnalysisChartsWidgetData() :
        analyser(nullptr), repository(nullptr), showMeasuresForCom(true)
    {}
};

StatisticalAnalysisChartsWidget::StatisticalAnalysisChartsWidget(CorpusRepository *repository, StatisticalAnalyserBase *analyser, QWidget *parent) :
    QWidget(parent), ui(new Ui::StatisticalAnalysisChartsWidget), d(new StatisticalAnalysisChartsWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;
    // Analyser
    d->analyser = analyser;

    // Measure combobox (start with measures for Communications by default)
    foreach (QString measureID, d->analyser->measureIDsForCommunication())
        ui->comboBoxMeasure->addItem(d->analyser->measureDefinition(measureID).displayName(), measureID);
    // Group by attributes
    if (d->repository->metadataStructure()) {
        ui->comboBoxGroupByCom->addItem("", "");
        ui->comboBoxGroupByCom->addItem("Communication ID", "ID");
        foreach (MetadataStructureAttribute *attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
            ui->comboBoxGroupByCom->addItem(attr->name(), attr->ID());
        }
        ui->comboBoxGroupByCom->setCurrentText("");
        // speaker
        ui->comboBoxGroupBySpk->addItem("", "");
        ui->comboBoxGroupBySpk->addItem("Speaker ID", "ID");
        foreach (MetadataStructureAttribute *attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Speaker)) {
            ui->comboBoxGroupBySpk->addItem(attr->name(), attr->ID());
        }
        ui->comboBoxGroupBySpk->setCurrentText("");
    }
    // Command Draw Chart
    connect(ui->commandDrawChart, &QAbstractButton::clicked, this, &StatisticalAnalysisChartsWidget::drawChart);
    // Defaults
    ui->checkBoxSetYMinMax->setChecked(true);
}

StatisticalAnalysisChartsWidget::~StatisticalAnalysisChartsWidget()
{
    delete ui;
    delete d;
}

void StatisticalAnalysisChartsWidget::showMeasuresForCom()
{
    d->showMeasuresForCom = true;
    ui->comboBoxMeasure->clear();
    if (d->analyser) {
        foreach (QString measureID, d->analyser->measureIDsForCommunication())
            ui->comboBoxMeasure->addItem(d->analyser->measureDefinition(measureID).displayName(), measureID);
    }
}

void StatisticalAnalysisChartsWidget::showMeasuresForSpk()
{
    d->showMeasuresForCom = false;
    ui->comboBoxMeasure->clear();
    if (d->analyser) {
        foreach (QString measureID, d->analyser->measureIDsForSpeaker())
            ui->comboBoxMeasure->addItem(d->analyser->measureDefinition(measureID).displayName(), measureID);
    }
}

void StatisticalAnalysisChartsWidget::drawChart()
{
    if (!d->analyser) return;

    // Get parameters from user interface
    QString measureID = ui->comboBoxMeasure->currentData().toString();
    QStringList groupAttributeIDsCom; groupAttributeIDsCom << ui->comboBoxGroupByCom->currentData().toString();
    QStringList groupAttributeIDsSpk; groupAttributeIDsSpk << ui->comboBoxGroupBySpk->currentData().toString();
    double yMin = ui->doubleSpinBoxYMin->value();
    double yMax = ui->doubleSpinBoxYMax->value();
    // Aggregate selected measure, over selected metadata attributes
    QMap<QString, QList<double> > aggregates;
    QString groupAttributes;
    if (d->showMeasuresForCom) {
        aggregates = d->analyser->aggregateMeasureCom(measureID, groupAttributeIDsCom);
        groupAttributes = ui->comboBoxGroupByCom->currentText();
    }
    else {
        aggregates = d->analyser->aggregateMeasureSpk(measureID, groupAttributeIDsCom, groupAttributeIDsSpk);
        QStringList sl; sl << ui->comboBoxGroupByCom->currentText() << ui->comboBoxGroupBySpk->currentText();
        groupAttributes = sl.join(", ");
    }
    if (groupAttributes.endsWith(", ")) groupAttributes.chop(2);
    // Create chart series
    QChart *chart = new QChart();
    double min(0), max(0);
    if (true) {
        QBoxPlotSeries *series = new QBoxPlotSeries();
        series->setName(measureID);
        foreach (QString groupID, aggregates.keys()) {
            QBoxSet *set = new QBoxSet(groupID);
            StatisticalSummary summary;
            summary.calculate(aggregates.value(groupID));
    //        set->setValue(QBoxSet::LowerExtreme, summary.firstQuartile() - 1.5 * summary.interQuartileRange());
    //        set->setValue(QBoxSet::UpperExtreme, summary.thirdQuartile() + 1.5 * summary.interQuartileRange());
            set->setValue(QBoxSet::LowerExtreme, summary.min());
            set->setValue(QBoxSet::UpperExtreme, summary.max());
            set->setValue(QBoxSet::Median, summary.median());
            set->setValue(QBoxSet::LowerQuartile, summary.firstQuartile());
            set->setValue(QBoxSet::UpperQuartile, summary.thirdQuartile());
            series->append(set);
            if (min > summary.min()) min = summary.min();
            if (max < summary.max()) max = summary.max();
        }
        chart->addSeries(series);
        chart->createDefaultAxes();
    } else {
        QBarSeries *series = new QBarSeries();
        series->setName(measureID);
        foreach (QString groupID, aggregates.keys()) {
            QBarSet *set = new QBarSet(groupID);
            StatisticalSummary summary;
            summary.calculate(aggregates.value(groupID));
            set->append(summary.mean());
            series->append(set);
            if (min > summary.min()) min = summary.min();
            if (max < summary.max()) max = summary.max();
        }
        chart->addSeries(series);
        chart->createDefaultAxes();
    }
    // Configure chart
    chart->setTitle(QString("%1 per %2").arg(ui->comboBoxMeasure->currentText(), groupAttributes));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    if (ui->checkBoxSetYMinMax->isChecked()) {
        chart->axes(Qt::Vertical).at(0)->setMin(yMin);
        chart->axes(Qt::Vertical).at(0)->setMax(yMax);
    } else {
        chart->axes(Qt::Vertical).at(0)->setMin(qRound(min * 0.9));
        chart->axes(Qt::Vertical).at(0)->setMax(qRound(max * 1.1));
    }
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    // Clear previous chart
    QLayoutItem *item;
    while ((item = ui->gridLayoutChart->takeAt(0)) != 0) { delete item; }
    QList<QChartView *> chartviews;
    chartviews = findChildren<QChartView *>();
    qDeleteAll(chartviews);
    // Create new chart view widget
    QChartView *chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    // Show chart view
    ui->gridLayoutChart->addWidget(chartView);
}
