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
};

PauseLengthDistributionWidget::PauseLengthDistributionWidget(CorpusRepository *repository, AnalyserTemporal *analyser, QWidget *parent) :
    QWidget(parent), ui(new Ui::PauseLengthDistributionWIdget), d(new PauseLengthDistributionWidgetData)
{
    ui->setupUi(this);
    d->repository = repository;
    d->analyser = analyser;
    // Defaults
    d->measureID = "Pause_SIL_Durations";
    d->numberOfBins = 10;
    d->minimumValue = 0;
    d->maximumValue = 1.0;
    // Draw charts command
    connect(ui->commandDraw, SIGNAL(clicked(bool)), this, SLOT(drawCharts()));
    d->groupAttributeIDsCom << "ID";
}

PauseLengthDistributionWidget::~PauseLengthDistributionWidget()
{
    delete ui;
    delete d;
}

void PauseLengthDistributionWidget::drawCharts()
{
    if (!d->analyser) return;
    if (!d->analyser->corpus()) return;

    QMap<QString, QList<double> > aggregates =
            d->analyser->aggregateMeasureSpk(d->measureID, d->groupAttributeIDsCom, d->groupAttributeIDsSpk);

    QBarSeries *series = new QBarSeries();
    QList<QBarSet *> barsets;
    foreach (QString groupID, aggregates.keys()) {
        HistogramCalculator hist;
        hist.setValues(aggregates.value(groupID));
        hist.setMinimum(d->minimumValue);
        hist.setMaximum(d->maximumValue);
        QBarSet *barset = new QBarSet(groupID);
        foreach (int count, hist.counts(d->numberOfBins)) barset->append(count);
        barsets << barset;
        series->append(barset);
    }
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString("%1").arg(AnalyserTemporalItem::measureDefinition(d->measureID).displayName()));
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
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->gridLayout->addWidget(chartView);

}

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
