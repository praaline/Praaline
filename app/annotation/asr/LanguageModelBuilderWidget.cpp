#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

#include "LanguageModelBuilderWidget.h"
#include "ui_LanguageModelBuilderWidget.h"

#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
using namespace Praaline::Core;

#include "PraalineASR/Sphinx/SphinxLanguageModelBuilder.h"
using namespace Praaline::ASR;

struct LanguageModelBuilderWidgetData {
    LanguageModelBuilderWidgetData() :
        languageModelBuilder(0)
    {}

    QPointer<CorpusRepository> repository;
    QPointer<Corpus> corpus;
    CorpusCommunication *communication;
    CorpusRecording *recording;
    CorpusAnnotation *annotation;

    SphinxLanguageModelBuilder *languageModelBuilder;
};

LanguageModelBuilderWidget::LanguageModelBuilderWidget(QWidget *parent) :
    ASRModuleWidgetBase(parent), ui(new Ui::LanguageModelBuilderWidget), d(new LanguageModelBuilderWidgetData())
{
    ui->setupUi(this);
    // Initialise
    d->languageModelBuilder = new SphinxLanguageModelBuilder(this);
    // Annotation Level and Attributes
    connect(ui->comboBoxUtterancesLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(utterancesLevelChanged(QString)));
    connect(ui->comboBoxTokensLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(tokensLevelChanged(QString)));
    // Command
    connect(ui->commandCreateLM, SIGNAL(clicked(bool)), this, SLOT(createLanguageModel()));
}

LanguageModelBuilderWidget::~LanguageModelBuilderWidget()
{
    delete ui;
    delete d;
}

void LanguageModelBuilderWidget::open(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot)
{
    if (!corpus) return;
    if (!com) return;
    if ((d->corpus == corpus) && (d->communication == com) && (d->recording == rec) && (d->annotation == annot)) return;

    if (d->repository != corpus->repository()) {
        if (!corpus->repository()) return;
        d->repository = corpus->repository();
        ui->comboBoxUtterancesLevel->clear();   ui->comboBoxUtterancesAttribute->clear();
        ui->comboBoxTokensLevel->clear();       ui->comboBoxTokensAttribute->clear();
        ui->comboBoxTokensLevel->addItem("", "");
        foreach (AnnotationStructureLevel *level, d->repository->annotationStructure()->levels()) {
            ui->comboBoxUtterancesLevel->addItem(level->name(), level->ID());
            ui->comboBoxTokensLevel->addItem(level->name(), level->ID());
        }
        utterancesLevelChanged("");
        tokensLevelChanged("");
    }

    d->corpus = corpus;
    d->communication = com;
    d->recording = rec;
    d->annotation = annot;
}

void LanguageModelBuilderWidget::utterancesLevelChanged(QString text)
{
    Q_UNUSED(text)
    if (!d->repository) return;
    QString levelID = ui->comboBoxUtterancesLevel->currentData().toString();
    AnnotationStructureLevel *level = d->repository->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxUtterancesAttribute->clear();
    ui->comboBoxUtterancesAttribute->addItem("(text)", "");
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxUtterancesAttribute->addItem(attribute->name(), attribute->ID());
    }
}

void LanguageModelBuilderWidget::tokensLevelChanged(QString text)
{
    Q_UNUSED(text)
    if (!d->repository) return;
    QString levelID = ui->comboBoxTokensLevel->currentData().toString();
    AnnotationStructureLevel *level = d->repository->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxTokensAttribute->clear();
    ui->comboBoxTokensAttribute->addItem("(text)", "");
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxTokensAttribute->addItem(attribute->name(), attribute->ID());
    }
}

void LanguageModelBuilderWidget::createLanguageModel()
{
    if (!d->communication) return;
    ui->textMessages->clear();
    d->languageModelBuilder->setUtterancesTier(ui->comboBoxUtterancesLevel->currentData().toString(),
                                               ui->comboBoxUtterancesAttribute->currentData().toString());
    d->languageModelBuilder->setTokensTier(ui->comboBoxTokensLevel->currentData().toString(),
                                           ui->comboBoxTokensAttribute->currentData().toString());
    QStringList normalisedUtterances;
    foreach (CorpusAnnotation *annot, d->communication->annotations()) {
         normalisedUtterances << d->languageModelBuilder->getNormalisedUtterances(annot);
    }
    foreach (QString utt, normalisedUtterances) {
        ui->textMessages->appendPlainText(utt);
    }

    // text2wfreq < S05_transcript.txt | wfreq2vocab > S05.vocab
    // text2idngram -vocab S05.vocab -idngram S05.idngram < S05_transcript.txt
    // idngram2lm -vocab_type 0 -idngram S05.idngram -vocab S05.vocab -arpa S05.lm
}
