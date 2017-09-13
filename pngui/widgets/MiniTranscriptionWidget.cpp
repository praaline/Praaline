#include <QString>
#include <QPointer>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include "MiniTranscriptionWidget.h"

#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

struct MiniTranscriptionWidgetData {
    MiniTranscriptionWidgetData() :
        skipPauses(true), transcriptionView(0), watcher(0)
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
    qDeleteAll(d->lines);
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

void MiniTranscriptionWidget::setAnnotation(QPointer<Praaline::Core::CorpusAnnotation> annot)
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

void MiniTranscriptionWidget::asyncCreateTranscript(QPointer<Praaline::Core::CorpusAnnotation> annot)
{
    static QMutex mutex;
    mutex.lock();
    QList<Interval *> intervals = annot->repository()->annotations()->getIntervals(
                AnnotationDatastore::Selection(annot->ID(), "", d->transcriptionLevelID));
    mutex.unlock();
    if (intervals.isEmpty()) return;
    QList<QTreeWidgetItem *> lines;
    foreach (Interval *intv, intervals) {
        if (!intv) continue;
        if (d->skipPauses && intv->isPauseSilent()) continue;
        QStringList fields;
        fields << QString::fromStdString(intv->tMin().toText())
               << intv->attribute("speakerID").toString()
               << intv->text();
        lines.append(new QTreeWidgetItem((QTreeWidget*)0, fields));
    }
    mutex.lock();
    if (d) d->lines.append(lines);
    mutex.unlock();
}

void MiniTranscriptionWidget::clear()
{
    d->transcriptionView->clear();
    if (d->watcher) d->watcher->cancel();
}

void MiniTranscriptionWidget::asyncCreateTranscriptFinished()
{
    d->transcriptionView->clear();
    d->transcriptionView->insertTopLevelItems(0, d->lines);
}

void MiniTranscriptionWidget::rebind(QPointer<Praaline::Core::CorpusAnnotation> annot, const QString &levelID)
{
    d->transcriptionView->clear();
    d->lines.clear();
    if (!annot) return;
    if (!annot->repository()) return;
    if (!annot->repository()->annotations()) return;
    if (!levelID.isEmpty()) d->transcriptionLevelID = levelID;
    d->annotation = annot;

    d->transcriptionView->insertTopLevelItem(0,
        new QTreeWidgetItem((QTreeWidget*)0, QStringList() << "Loading..." << "Loading..." << "Loading..."));
    d->watcher = QSharedPointer<QFutureWatcher<void> >(new QFutureWatcher<void>());
    connect(d->watcher.data(), SIGNAL(finished()), this, SLOT(asyncCreateTranscriptFinished()));

    QFuture<void> future = QtConcurrent::run(this, &MiniTranscriptionWidget::asyncCreateTranscript, annot);
    d->watcher->setFuture(future);
}
