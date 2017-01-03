#include <QString>
#include <QPointer>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QStandardItemModel>
#include "MiniTranscriptionWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusAnnotation.h"
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
    d->transcriptionLevelID = levelID;
}

void MiniTranscriptionWidget::setAnnotation(QPointer<Praaline::Core::CorpusAnnotation> annot)
{
    d->transcriptionView->clear();
    d->lines.clear();
    if (!annot) return;
    QPointer<Corpus> corpus = annot->corpus();
    if (!corpus) return;
    foreach (Interval *intv, corpus->datastoreAnnotations()->getIntervals(
                 AnnotationDatastore::Selection(annot->ID(), "", d->transcriptionLevelID))) {
        QStringList fields;
        fields << QString::fromStdString(intv->tMin().toText())
               << intv->attribute("speakerID").toString()
               << intv->text();
        d->lines.append(new QTreeWidgetItem((QTreeWidget*)0, fields));
    }
    d->transcriptionView->insertTopLevelItems(0, d->lines);
}
