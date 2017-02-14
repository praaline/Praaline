#include <QDebug>
#include <QMap>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
QT_CHARTS_USE_NAMESPACE

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/statistics/HistogramCalculator.h"
using namespace Praaline::Core;

#include "pngui/FlowLayout.h"

#include "AnalyserTemporal.h"

#include "PauseLengthDistributionWIdget.h"
#include "ui_PauseLengthDistributionWIdget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

struct PauseLengthDistributionWidgetData {
    PauseLengthDistributionWidgetData() :
        repository(0), analyser(0), numberOfBins(0), minimumValue(0), maximumValue(0)
    {}

    CorpusRepository *repository;
    AnalyserTemporal *analyser;
    QString measureID;
    int numberOfBins;
    double minimumValue;
    double maximumValue;
    QStringList groupAttributeIDsCom;
    QStringList groupAttributeIDsSpk;
    QMap<QString, QList<double> > aggregates;
};

PauseLengthDistributionWidget::PauseLengthDistributionWidget(CorpusRepository *repository, AnalyserTemporal *analyser, QWidget *parent) :
    QWidget(parent), ui(new Ui::PauseLengthDistributionWIdget), d(new PauseLengthDistributionWidgetData)
{
    ui->setupUi(this);
    d->repository = repository;
    d->analyser = analyser;
    // Defaults
    d->measureID = "Pause_SIL_Durations";
    d->numberOfBins = 20;
    d->minimumValue = 0;
    d->maximumValue = 1.0;
    // Draw charts command
    connect(ui->commandDraw, SIGNAL(clicked(bool)), this, SLOT(drawCharts()));
    d->groupAttributeIDsCom << "genre";
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
        foreach (int count, hist.counts(d->numberOfBins)) barset->append(count);
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

    QList<QChart *> charts;
    int chartsPerColumn = 3;
    int i = 0;
    foreach (QString groupID, groups.keys()) {
        QChart *chart = drawHistogram(groupID, groups.value(groupID));
        if (chart) charts << chart;

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        ui->gridLayoutCharts->addWidget(chartView, i / chartsPerColumn, i % chartsPerColumn);
        i++;
    }

}

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
