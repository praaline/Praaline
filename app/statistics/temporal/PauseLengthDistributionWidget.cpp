#include <QDebug>
#include <QMap>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
QT_CHARTS_USE_NAMESPACE

#include "pncore/corpus/Corpus.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/statistics/HistogramCalculator.h"
using namespace Praaline::Core;

#include "pngui/FlowLayout.h"

#include "AnalyserTemporal.h"

#include "PauseLengthDistributionWidget.h"
#include "ui_PauseLengthDistributionWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

struct PauseLengthDistributionWidgetData {
    PauseLengthDistributionWidgetData() :
        repository(0), analyser(0), numberOfBins(0), minimumValue(0), maximumValue(0), maximumCount(10)
    {}

    CorpusRepository *repository;
    AnalyserTemporal *analyser;
    QString measureID;
    int numberOfBins;
    double minimumValue;
    double maximumValue;
    int maximumCount;
    QStringList groupAttributeIDsCom;
    QStringList groupAttributeIDsSpk;
    QMap<QString, QList<double> > aggregates;
    QList<QChart *> charts;
};

PauseLengthDistributionWidget::PauseLengthDistributionWidget(CorpusRepository *repository, AnalyserTemporal *analyser, QWidget *parent) :
    QWidget(parent), ui(new Ui::PauseLengthDistributionWidget), d(new PauseLengthDistributionWidgetData)
{
    ui->setupUi(this);
    d->repository = repository;
    d->analyser = analyser;
    if (!analyser) return;
    // Defaults
    d->measureID = "Pause_SIL_Durations";
    d->numberOfBins = 10;
    d->minimumValue = 0;
    d->maximumValue = 1.0;
    // Measure combobox
    ui->comboBoxMeasure->addItems(AnalyserTemporalItem::vectorMeasureIDsForSpeaker());
    ui->comboBoxMeasure->setCurrentText(d->measureID);
    // Group by attributes
    if (d->repository->metadataStructure()) {
        ui->comboBoxGroupByCom->addItem("", "");
        ui->comboBoxGroupByCom->addItem("Communication ID", "ID");
        foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
            ui->comboBoxGroupByCom->addItem(attr->name(), attr->ID());
        }
        ui->comboBoxGroupByCom->setCurrentText("");
        // speaker
        ui->comboBoxGroupBySpk->addItem("", "");
        ui->comboBoxGroupBySpk->addItem("Speaker ID", "ID");
        foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Speaker)) {
            ui->comboBoxGroupBySpk->addItem(attr->name(), attr->ID());
        }
        ui->comboBoxGroupBySpk->setCurrentText("");
    }
    // Histogram
    ui->spinBoxNumberOfBins->setValue(d->numberOfBins);
    ui->doubleSpinBoxMinimum->setValue(d->minimumValue);
    ui->doubleSpinBoxMaximum->setValue(d->maximumValue);
    // Number of columns
    ui->spinBoxNumberOfColumns->setMinimum(1);
    ui->spinBoxNumberOfColumns->setValue(1);
    // Draw charts command
    connect(ui->commandDraw, SIGNAL(clicked(bool)), this, SLOT(drawCharts()));
}

PauseLengthDistributionWidget::~PauseLengthDistributionWidget()
{
    delete ui;
    delete d;
}

QChart *PauseLengthDistributionWidget::drawHistogram(const QString &title, QMap<QString, QList<double> > aggregates)
{
    if (!d->analyser) return Q_NULLPTR;
    if (!d->analyser->corpus()) return Q_NULLPTR;

    QBarSeries *series = new QBarSeries();
    QList<QBarSet *> barsets;
    foreach (QString aggregateID, aggregates.keys()) {
        HistogramCalculator hist;
        hist.setValues(aggregates.value(aggregateID));
        hist.setMinimum(d->minimumValue);
        hist.setMaximum(d->maximumValue);
        QBarSet *barset = new QBarSet(aggregateID);
        foreach (int count, hist.counts(d->numberOfBins)) {
            barset->append(count);
            if (count > d->maximumCount) d->maximumCount = count;
        }
        barsets << barset;
        series->append(barset);
    }
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    // Bin edges + labels on the x axis
    HistogramCalculator hist;
    hist.setMinimum(0); hist.setMaximum(2.0);
    QStringList binLabels;
    foreach (double edge, hist.binEdges(d->numberOfBins)) binLabels << QString("%1").arg(edge);
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(binLabels);
    chart->createDefaultAxes();
    chart->setAxisX(axis, series);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    return chart;
}

void PauseLengthDistributionWidget::drawCharts()
{
    if (!d->analyser) return;
    if (!d->analyser->corpus()) return;

    // Get parameters from user interface
    d->measureID = ui->comboBoxMeasure->currentText();
    d->groupAttributeIDsCom.clear();
    d->groupAttributeIDsCom << ui->comboBoxGroupByCom->currentData().toString();
    d->groupAttributeIDsSpk.clear();
    d->groupAttributeIDsSpk << ui->comboBoxGroupBySpk->currentData().toString();
    d->numberOfBins = ui->spinBoxNumberOfBins->value();
    d->minimumValue = ui->doubleSpinBoxMinimum->value();
    d->maximumValue = ui->doubleSpinBoxMaximum->value();
    int numberOfColumns = ui->spinBoxNumberOfColumns->value();

    d->aggregates = d->analyser->aggregateMeasureSpk(d->measureID, d->groupAttributeIDsCom, d->groupAttributeIDsSpk);

    // Create groups if necessary
    QMap<QString, QMap<QString, QList<double> > > groups;
    foreach (QString key, d->aggregates.keys()) {
        if (key.contains("::")) {
            QString groupID = key.section("::", 0, 0);
            QString subgroupID = key.section("::", 1);
            QList<double> data = d->aggregates.value(key);
            if (groups.contains(groupID)) {
                groups[groupID].insert(subgroupID, data);
            } else {
                groups.insert(groupID, QMap<QString, QList<double> >());
                groups[groupID].insert(subgroupID, data);
            }
        } else {
            groups.insert(key, QMap<QString, QList<double> >());
            groups[key].insert(key, d->aggregates.value(key));
        }
    }

    // Clear layout, delete chart view widgets
    QLayoutItem *item;
    while ((item = ui->gridLayoutCharts->takeAt(0)) != 0) { delete item; }
    QList<QChartView *> chartviews;
    chartviews = findChildren<QChartView *>();
    qDeleteAll(chartviews);
    d->charts.clear();

    int i = 0;
    ui->gridLayoutCharts->setSizeConstraint(QLayout::SetNoConstraint);
    int size = ui->scrollAreaWidgetContents->width() / numberOfColumns - 10;

    d->maximumCount = 10; // reset
    foreach (QString groupID, groups.keys()) {
        QChart *chart = drawHistogram(groupID, groups.value(groupID));
        if (chart) d->charts << chart;

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        chartView->setMinimumSize(size, size);

        ui->gridLayoutCharts->addWidget(chartView, i / numberOfColumns, i % numberOfColumns);
        i++;
    }
    foreach (QChart *chart, d->charts) {
        if (ui->checkBoxSetYMax->isChecked())
            chart->axisY()->setMax(ui->doubleSpinBoxYMax->value());
        else
            chart->axisY()->setMax(d->maximumCount);
    }

    chartviews = findChildren<QChartView *>();
    foreach (QChartView *chartview, chartviews) {
        chartview->resize(size, size);
    }

}

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
