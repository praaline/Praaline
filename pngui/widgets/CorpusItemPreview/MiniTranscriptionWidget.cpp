#include <QString>
#include <QPointer>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>

#include "PraalineCore/Corpus/CorpusAnnotation.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "MiniTranscriptionWidget.h"


struct MiniTranscriptionWidgetData {
    MiniTranscriptionWidgetData() :
        skipPauses(true), annotation(nullptr), transcriptionView(nullptr), watcher(nullptr)
    {}

    bool skipPauses;
    QString transcriptionLevelID;
    QPointer<CorpusAnnotation> annotation;
    QTreeWidget *transcriptionView;
    QList<QTreeWidgetItem *> lines;
    QSharedPointer<QFutureWatcher<void> > watcher;
};

MiniTranscriptionWidget::MiniTranscriptionWidget(QWidget *parent) :
    QWidget(parent), d(new MiniTranscriptionWidgetData)
{
    QStringList headerLabels;
    headerLabels << tr("Time") << tr("Speaker") << tr("Transcript");

    // Default levelID for a transcription
    d->transcriptionLevelID = "transcription";

    d->transcriptionView = new QTreeWidget(this);
    d->transcriptionView->setHeaderLabels(headerLabels);
    d->transcriptionView->setWordWrap(true);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(d->transcriptionView);
    layout->setMargin(0);
    setLayout(layout);
}

MiniTranscriptionWidget::~MiniTranscriptionWidget()
{
    // qDeleteAll(d->lines);
    delete d;
}

void MiniTranscriptionWidget::setTranscriptionLevelID(const QString &levelID)
{
    if (d->transcriptionLevelID == levelID) return;
    rebind(d->annotation, levelID);
}

QString MiniTranscriptionWidget::transcriptionLevelID() const
{
    return d->transcriptionLevelID;
}

void MiniTranscriptionWidget::setAnnotation(Praaline::Core::CorpusAnnotation *annot)
{
    if (d->annotation == annot) return;
    rebind(annot, d->transcriptionLevelID);
}

bool MiniTranscriptionWidget::skipPauses() const
{
    return d->skipPauses;
}

void MiniTranscriptionWidget::setSkipPauses(bool skip)
{
    if (d->skipPauses == skip) return;
    d->skipPauses = skip;
    rebind(d->annotation, d->transcriptionLevelID);
}

void MiniTranscriptionWidget::asyncCreateTranscript(Praaline::Core::CorpusAnnotation *annot)
{
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    if (!annot) return;
    if (!annot->repository()) return;
    if (!annot->repository()->annotations()) return;
    QList<Interval *> intervals = annot->repository()->annotations()->getIntervals(
                AnnotationDatastore::Selection(annot->ID(), "", d->transcriptionLevelID));
    if (intervals.isEmpty()) return;
    QList<QTreeWidgetItem *> lines;
    foreach (Interval *intv, intervals) {
        if (!intv) continue;
        if (d->skipPauses && intv->isPauseSilent()) continue;
        QStringList fields;
        fields << QString::fromStdString(intv->tMin().toText())
               << intv->attribute("speakerID").toString()
               << intv->text();
        lines.append(new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), fields));
    }
    if (d) d->lines.append(lines);
}

void MiniTranscriptionWidget::clear()
{
    if (d->transcriptionView) d->transcriptionView->clear();
    if (d->watcher) d->watcher->cancel();
}

void MiniTranscriptionWidget::asyncCreateTranscriptFinished()
{
    d->transcriptionView->clear();
    d->transcriptionView->insertTopLevelItems(0, d->lines);
}

void MiniTranscriptionWidget::rebind(Praaline::Core::CorpusAnnotation *annot, const QString &levelID)
{
    if (!d->transcriptionView) return;
    d->transcriptionView->clear();
    d->lines.clear();
    if (!annot) return;
    if (!annot->repository()) return;
    if (!annot->repository()->annotations()) return;
    if (!levelID.isEmpty()) d->transcriptionLevelID = levelID;
    d->annotation = annot;

    d->transcriptionView->insertTopLevelItem(0,
        new QTreeWidgetItem(QStringList() << "Loading..." << "Loading..." << "Loading..."));
    d->watcher = QSharedPointer<QFutureWatcher<void> >(new QFutureWatcher<void>());
    connect(d->watcher.data(), &QFutureWatcherBase::finished, this, &MiniTranscriptionWidget::asyncCreateTranscriptFinished);

    QFuture<void> future = QtConcurrent::run(this, &MiniTranscriptionWidget::asyncCreateTranscript, annot);
    d->watcher->setFuture(future);
}
