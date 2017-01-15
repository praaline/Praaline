#include <QString>
#include <QPointer>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QStandardItemModel>
#include "MiniTranscriptionWidget.h"

#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

struct MiniTranscriptionWidgetData {
    MiniTranscriptionWidgetData() :
        transcriptionView(0)
    {}

    QString transcriptionLevelID;
    QPointer<CorpusAnnotation> annotation;
    QTreeWidget *transcriptionView;
    QList<QTreeWidgetItem *> lines;
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

void MiniTranscriptionWidget::rebind(QPointer<Praaline::Core::CorpusAnnotation> annot, const QString &levelID)
{
    d->transcriptionView->clear();
    d->lines.clear();
    if (!annot) return;
    if (!annot->repository()) return;
    if (!annot->repository()->annotations()) return;
    if (!levelID.isEmpty()) d->transcriptionLevelID = levelID;
    foreach (Interval *intv, annot->repository()->annotations()->getIntervals(
                 AnnotationDatastore::Selection(annot->ID(), "", d->transcriptionLevelID))) {
        QStringList fields;
        fields << QString::fromStdString(intv->tMin().toText())
               << intv->attribute("speakerID").toString()
               << intv->text();
        d->lines.append(new QTreeWidgetItem((QTreeWidget*)0, fields));
    }
    d->transcriptionView->insertTopLevelItems(0, d->lines);
    d->annotation = annot;
}
