#include "SpeechRecognitionWidget.h"
#include "ui_SpeechRecognitionWidget.h"

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/CorpusBookmarkModel.h"
#include "pngui/widgets/CorpusItemSelectorWidget.h"

#include "asr/ASRModuleWidgetBase.h"
#include "asr/AutomaticTranscriptionWidget.h"
#include "asr/LongSoundAlignerWidget.h"

struct SpeechRecognitionWidgetData {
    SpeechRecognitionWidgetData() :
        corpusItemSelector(0)
    {}

    // Corpus item selector
    CorpusItemSelectorWidget *corpusItemSelector;
    // Open editors
    QList<ASRModuleWidgetBase *> openModules;
};

SpeechRecognitionWidget::SpeechRecognitionWidget(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::SpeechRecognitionWidget), d(new SpeechRecognitionWidgetData)
{
    ui->setupUi(this);

    // Module Selector
    ui->comboBoxModuleSelection->addItem("Automatic Transcription", "AutomaticTranscription");
    ui->comboBoxModuleSelection->addItem("Long Sound Alignment", "LongSoundAligner");
    ui->comboBoxModuleSelection->setCurrentIndex(0);
    connect(ui->commandOpenModule, SIGNAL(clicked(bool)), this, SLOT(moduleTabNew()));

    // Corpus item selector
    d->corpusItemSelector = new CorpusItemSelectorWidget(this);
    ui->gridLayoutCorpusExplorer->addWidget(d->corpusItemSelector);
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusCommunication(QPointer<Corpus>,QPointer<CorpusCommunication>)),
            this, SLOT(selectedCorpusCommunication(QPointer<Corpus>,QPointer<CorpusCommunication>)));
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusRecording(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusRecording>)),
            this, SLOT(selectedCorpusRecording(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusRecording>)));
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusAnnotation(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusAnnotation>)),
            this, SLOT(selectedCorpusAnnotation(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusAnnotation>)));

    // Left-right splitter sizes
    ui->splitterLR->setSizes(QList<int>() << 50 << 350);

    // Handle close document
    connect(ui->tabWidgetModules, SIGNAL(tabCloseRequested(int)), this, SLOT(moduleTabCloseRequested(int)));
}

SpeechRecognitionWidget::~SpeechRecognitionWidget()
{
    delete ui;
    delete d;
}

// ====================================================================================================================
// Corpus item selection (selector -> editor)
// ====================================================================================================================

void SpeechRecognitionWidget::selectedCorpusCommunication(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    QPointer<CorpusRecording> rec(0);
    QPointer<CorpusAnnotation> annot(0);
    if (!corpus) return;
    if (!com) return;
    if (com->recordingsCount() >= 1) rec = com->recordings().first();
    if (com->annotationsCount() >= 1) annot = com->annotations().first();
    selectionChanged(corpus, com, rec, annot);
}

void SpeechRecognitionWidget::selectedCorpusRecording(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                     QPointer<CorpusRecording> rec)
{
    QPointer<CorpusAnnotation> annot(0);
    if (!corpus) return;
    if (!com) return;
    if (!rec) return;
    if (com->annotationsCount() == 1) {
        annot = com->annotations().first();
    } else if (com->annotationsCount() > 0) {
        annot = com->annotation(rec->ID());
    }
    selectionChanged(corpus, com, rec, annot);
}

void SpeechRecognitionWidget::selectedCorpusAnnotation(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                      QPointer<CorpusAnnotation> annot)
{
    QPointer<CorpusRecording> rec(0);
    if (!corpus) return;
    if (!com) return;
    if (!annot) return;
    if (com->recordingsCount() == 1) {
        rec = com->recordings().first();
    } else if (com->annotationsCount() > 0) {
        rec = com->recording(annot->ID());
    }
    selectionChanged(corpus, com, rec, annot);
}

void SpeechRecognitionWidget::selectionChanged(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                              QPointer<CorpusRecording> rec, QPointer<CorpusAnnotation> annot)
{
    int index = ui->tabWidgetModules->currentIndex();
    if (index < 0 || index >= d->openModules.count()) return;
    if (ui->checkBoxFollowSelection->isChecked()) {
        d->openModules.at(index)->open(corpus, com, rec, annot);
    }
}

void SpeechRecognitionWidget::moduleTabNew()
{
    ASRModuleWidgetBase *module(0);
    QString moduleName = ui->comboBoxModuleSelection->currentData().toString();
    if      (moduleName == "AutomaticTranscription") {
        module = new AutomaticTranscriptionWidget(this);
    }
    else if (moduleName == "LongSoundAligner") {
        module = new LongSoundAlignerWidget(this);
    }
    if (!module) return;
    d->openModules.append(module);
    ui->tabWidgetModules->addTab(module, ui->comboBoxModuleSelection->currentText());
    ui->tabWidgetModules->setCurrentWidget(module);
}

void SpeechRecognitionWidget::moduleTabCloseRequested(int index)
{
    ASRModuleWidgetBase *module = d->openModules.takeAt(index);
    if (module) {
        module->deleteLater();
        ui->tabWidgetModules->removeTab(index);
    }
}

