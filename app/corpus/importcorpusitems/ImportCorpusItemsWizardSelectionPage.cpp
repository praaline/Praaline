#include <QFileDialog>
#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/FileDatastore.h"
#include "ImportCorpusItemsWizardSelectionPage.h"
#include "ui_ImportCorpusItemsWizardSelectionPage.h"

struct ImportCorpusItemsWizardSelectionPageData {
    ImportCorpusItemsWizardSelectionPageData(QPointer<Corpus> corpus,
                                             QMap<QPair<QString, QString>, CorpusRecording *> &candidateRecordings,
                                             QMap<QPair<QString, QString>, CorpusAnnotation *> &candidateAnnotations) :
        corpus(corpus), candidateRecordings(candidateRecordings), candidateAnnotations(candidateAnnotations),
        abort(false)
    {}

    QPointer<Corpus> corpus;
    QMap<QPair<QString, QString>, CorpusRecording *> &candidateRecordings;
    QMap<QPair<QString, QString>, CorpusAnnotation *> &candidateAnnotations;
    QPointer<QStandardItemModel> modelFormatsRecording;
    QPointer<QStandardItemModel> modelFormatsAnnotation;
    bool abort;
};

ImportCorpusItemsWizardSelectionPage::ImportCorpusItemsWizardSelectionPage(
        QPointer<Corpus> corpus,
        QMap<QPair<QString, QString>, CorpusRecording *> &candidateRecordings,
        QMap<QPair<QString, QString>, CorpusAnnotation *> &candidateAnnotations,
        QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ImportCorpusItemsWizardSelectionPage),
    d(new ImportCorpusItemsWizardSelectionPageData(corpus, candidateRecordings, candidateAnnotations))
{
    ui->setupUi(this);
    connect(ui->commandSelectFolder, SIGNAL(clicked()), this, SLOT(selectFolder()));
    connect(ui->commandAbort, SIGNAL(clicked()), this, SLOT(abortProcess()));

    // Set path
    if (corpus && corpus->repository() && corpus->repository()->files())
        ui->editFolderName->setText(corpus->repository()->files()->basePath());

    // Check lists for file formats
    d->modelFormatsRecording = new QStandardItemModel(this);
    d->modelFormatsAnnotation = new QStandardItemModel(this);
    d->modelFormatsRecording->setColumnCount(1);
    d->modelFormatsAnnotation->setColumnCount(1);
    QStandardItem *item;
    // Media formats
    item = new QStandardItem("Wave files (.wav)");  item->setData("wav");
    item->setCheckable(true);   item->setCheckState(Qt::Checked);
    d->modelFormatsRecording->setItem(0, 0, item);
    item = new QStandardItem("MP3 files (.mp3)");  item->setData("mp3");
    item->setCheckable(true);   item->setCheckState(Qt::Checked);
    d->modelFormatsRecording->setItem(1, 0, item);
    ui->treeviewRecordingsFormat->setModel(d->modelFormatsRecording);
    ui->treeviewRecordingsFormat->setHeaderHidden(true);
    // Annotation formats
    item = new QStandardItem("Praat textgrid files (.textgrid)");  item->setData("textgrid");
    item->setCheckable(true);   item->setCheckState(Qt::Checked);
    d->modelFormatsAnnotation->setItem(0, 0, item);
    item = new QStandardItem("TranscriberAG files (.trs)");  item->setData("trs");
    item->setCheckable(true);   item->setCheckState(Qt::Checked);
    d->modelFormatsAnnotation->setItem(1, 0, item);
    item = new QStandardItem("SubRip subtitle files (.srt)");  item->setData("srt");
    item->setCheckable(true);   item->setCheckState(Qt::Checked);
    d->modelFormatsAnnotation->setItem(2, 0, item);

    ui->treeviewAnnotationsFormat->setModel(d->modelFormatsAnnotation);
    ui->treeviewAnnotationsFormat->setHeaderHidden(true);

    QStringList filters;
    filters << "no filter" << "contains" << "does not contain" << "starts with" << "does not start with"
            << "ends with" << "does not end with";
    ui->comboAnnotationsFilter->addItems(filters);
    ui->comboRecordingsFilter->addItems(filters);

    setTitle("Add Corpus Items from folder");
    setSubTitle("In this first step you may select the folder that contains your corpus files. You may select filters to help Praaline select only "
                "those files you want included in your corpus, and how to infer the corpus Communication ID codes from the filenames.");
}

ImportCorpusItemsWizardSelectionPage::~ImportCorpusItemsWizardSelectionPage()
{
    delete ui;
}

void ImportCorpusItemsWizardSelectionPage::selectFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory to Process"), ui->editFolderName->text(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty())
        ui->editFolderName->setText(directory);
}

void ImportCorpusItemsWizardSelectionPage::abortProcess()
{
    d->abort = true;
}

bool ImportCorpusItemsWizardSelectionPage::validatePage ()
{
    ui->stackedWidget->setCurrentIndex(1);
    qDeleteAll(d->candidateRecordings);
    d->candidateRecordings.clear();
    qDeleteAll(d->candidateAnnotations);
    d->candidateAnnotations.clear();
    ui->texteditMessages->clear();
    d->abort = false;

    QDir dirinfo(ui->editFolderName->text());
    QStringList recordingFilters, annotationFilters;
    for (int i = 0; i < d->modelFormatsRecording->rowCount(); ++i) {
        QStandardItem *item = d->modelFormatsRecording->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            QString suffix = item->data().toString().toLower();
            recordingFilters << QString("*.") + suffix;
        }
    }
    for (int i = 0; i < d->modelFormatsAnnotation->rowCount(); ++i) {
        QStandardItem *item = d->modelFormatsAnnotation->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            QString suffix = item->data().toString();
            annotationFilters << QString("*.") + suffix;
        }
    }

    if (!recordingFilters.isEmpty()) {
        addFiles(dirinfo.canonicalPath(), recordingFilters, ui->checkBoxRecursive->isChecked(), true);
        if (d->abort) {
            ui->stackedWidget->setCurrentIndex(0);
            return false;
        }
    }
    if (!annotationFilters.isEmpty()) {
        addFiles(dirinfo.canonicalPath(), annotationFilters, ui->checkBoxRecursive->isChecked(), false);
        if (d->abort) {
            ui->stackedWidget->setCurrentIndex(0);
            return false;
        }
    }
    ui->stackedWidget->setCurrentIndex(0);
    return true;
}

// private
void ImportCorpusItemsWizardSelectionPage::addFiles(const QString& directory, const QStringList &filters, bool recursive,
                                                    bool isRecording)
{
    if (d->abort) return;
    QDir dirinfo(directory);
    QFileInfoList list;
    list << dirinfo.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
    dirinfo.setNameFilters(filters);
    list << dirinfo.entryInfoList();

    int i = 0;
    foreach (QFileInfo info, list) {
        ui->progressBar->setMaximum(list.count());
        if (info.isDir()) {
            if (recursive) {
                addFiles(info.filePath(), filters, recursive, isRecording);
            }
        }
        else {
            processFile(info, isRecording);
            ++i;
            ui->progressBar->setValue(i);
            QApplication::processEvents();
        }
    }
}

// private
bool ImportCorpusItemsWizardSelectionPage::processFile(const QFileInfo &info, bool isRecording)
{
    if (d->abort) return false;
    bool isAnnotation = !isRecording;
    if (!d->modelFormatsRecording) return false;
    if (!d->modelFormatsAnnotation) return false;

    // check filter
    QString filterOperator, filter;
    if (isRecording) {
        filterOperator = ui->comboRecordingsFilter->currentText();
        filter = ui->editRecordingsFilter->text();
    }
    else if (isAnnotation) {
        filterOperator = ui->comboAnnotationsFilter->currentText();
        filter = ui->editAnnotationsFilter->text();
    }
    QString baseFilename = info.baseName();
    if (filterOperator == "contains") {
        if (!baseFilename.contains(filter, Qt::CaseInsensitive)) return false;
    }
    else if (filterOperator == "does not contain") {
        if (baseFilename.contains(filter, Qt::CaseInsensitive)) return false;
    }
    else if (filterOperator == "starts with") {
        if (!baseFilename.startsWith(filter, Qt::CaseInsensitive)) return false;
    }
    else if (filterOperator == "does not start with") {
        if (baseFilename.startsWith(filter, Qt::CaseInsensitive)) return false;
    }
    else if (filterOperator == "ends with") {
        if (!baseFilename.endsWith(filter, Qt::CaseInsensitive)) return false;
    }
    else if (filterOperator == "does not end with") {
        if (baseFilename.endsWith(filter, Qt::CaseInsensitive)) return false;
    }
    // now process file
    QString communicationID = communicationIDFromFilename(info);
    if (communicationID.isEmpty()) return false;
    if (isRecording) {
        CorpusRecording *rec = new CorpusRecording(baseFilename);
        rec->setName(baseFilename);
        rec->setFilename(d->corpus->repository()->files()->getRelativeToBasePath(info.canonicalFilePath()));
        d->candidateRecordings.insert(QPair<QString, QString>(communicationID, baseFilename), rec);
        ui->texteditMessages->appendPlainText(QString("MEDIA RECORDING %1 >> Communication ID: %2, Recording ID: %3")
                                              .arg(info.canonicalFilePath()).arg(communicationID).arg(baseFilename));
        ui->texteditMessages->moveCursor(QTextCursor::End);
    } else if (isAnnotation) {
        CorpusAnnotation *annot = new CorpusAnnotation(communicationID); // baseFilename
        annot->setFilename(info.canonicalFilePath());
        // Speaker Policy
        if (ui->optionSpeakerSingle->isChecked()) {
            annot->setProperty("speakerPolicy", ImportAnnotations::SpeakerPolicySingle);
            annot->setProperty("speakerPolicyData", speakerIDFromFilename(info));
        }
        else if (ui->optionSpeakerTiers->isChecked()) {
            annot->setProperty("speakerPolicy", ImportAnnotations::SpeakerPolicyTierNames);
            annot->setProperty("speakerPolicyData", "");
        }
        else if (ui->optionSpeakerIntervals->isChecked()) {
            annot->setProperty("speakerPolicy", ImportAnnotations::SpeakerPolicyIntervals);
            annot->setProperty("speakerPolicyData", "speaker");
        }
        else if (ui->optionSpeakerPrimaryAndSecondary->isChecked()) {
            annot->setProperty("speakerPolicy", ImportAnnotations::SpeakerPolicyPrimaryAndSecondary);
            annot->setProperty("speakerPolicyData", "ortho;loc2");
        }

        d->candidateAnnotations.insert(QPair<QString, QString>(communicationID, baseFilename), annot);
        ui->texteditMessages->appendPlainText(QString("ANNOTATION %1 >> Communication ID: %2, Annotation ID: %3")
                                              .arg(info.canonicalFilePath()).arg(communicationID).arg(baseFilename));
        ui->texteditMessages->moveCursor(QTextCursor::End);
    }
    return true;
}

// private
QString ImportCorpusItemsWizardSelectionPage::communicationIDFromFilename(const QFileInfo &fileInfo)
{
    QString name = fileInfo.baseName();
    if (ui->optionComIDSubfolderName->isChecked()) {
        QString path = fileInfo.canonicalFilePath();
        QStringList folders = path.split("/");
        // canoncial file path = /abc/def/xyz/file.ext => we need the second-to-last item
        if (folders.count() > 1)
            name = folders.at(folders.count() - 2);
    }
    // Process name
    // Trim left
    if (ui->optionComIDTakeLeft->isChecked()) {
        name = name.mid(0, ui->spinBoxComIDLeftChars->value());
    }
    else if (ui->optionComIDRemoveLeft->isChecked()) {
        name = name.mid(ui->spinBoxComIDLeftChars->value(), -1);
    }
    // Trim right
    if (ui->optionComIDTakeRight->isChecked()) {
        name = name.right(ui->spinBoxComIDRightChars->value());
    }
    else if (ui->optionComIDRemoveRight->isChecked()) {
        if (name.length() > ui->spinBoxComIDRightChars->value())
            name = name.mid(0, name.length() - ui->spinBoxComIDRightChars->value());
    }
    // Remove left and right
    if (!ui->editComIDRemoveFromBeginning->text().isEmpty()) {
        QStringList stringsToRemove = ui->editComIDRemoveFromBeginning->text().split(",");
        foreach (QString stringToRemove, stringsToRemove) {
            if (name.startsWith(stringToRemove.trimmed()))
                name = name.mid(stringToRemove.trimmed().length(), -1);
        }
    }
    if (!ui->editComIDRemoveFromEnd->text().isEmpty()) {
        QStringList stringsToRemove = ui->editComIDRemoveFromEnd->text().split(",");
        foreach (QString stringToRemove, stringsToRemove) {
            if (name.endsWith(stringToRemove.trimmed()))
                name.chop(stringToRemove.trimmed().length());
        }
    }
    return name;
}

// private
QString ImportCorpusItemsWizardSelectionPage::speakerIDFromFilename(const QFileInfo &fileInfo)
{
    QString name = fileInfo.baseName();
    // Process name
    // Trim left
    if (ui->optionSpkIDTakeLeft->isChecked()) {
        name = name.mid(0, ui->spinBoxSpkIDLeftChars->value());
    }
    else if (ui->optionSpkIDRemoveLeft->isChecked()) {
        name = name.mid(ui->spinBoxSpkIDLeftChars->value(), -1);
    }
    // Trim right
    if (ui->optionSpkIDTakeRight->isChecked()) {
        name = name.right(ui->spinBoxSpkIDRightChars->value());
    }
    else if (ui->optionSpkIDRemoveRight->isChecked()) {
        if (name.length() > ui->spinBoxSpkIDRightChars->value())
            name = name.mid(0, name.length() - ui->spinBoxSpkIDRightChars->value());
    }
    // Remove left and right
    if (!ui->editSpkIDRemoveFromBeginning->text().isEmpty()) {
        QStringList stringsToRemove = ui->editSpkIDRemoveFromBeginning->text().split(",");
        foreach (QString stringToRemove, stringsToRemove) {
            if (name.startsWith(stringToRemove.trimmed()))
                name = name.mid(stringToRemove.trimmed().length(), -1);
        }
    }
    if (!ui->editSpkIDRemoveFromEnd->text().isEmpty()) {
        QStringList stringsToRemove = ui->editSpkIDRemoveFromEnd->text().split(",");
        foreach (QString stringToRemove, stringsToRemove) {
            if (name.endsWith(stringToRemove.trimmed()))
                name.chop(stringToRemove.trimmed().length());
        }
    }
    return name;
}

