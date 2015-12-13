#include <QFileDialog>
#include "importcorpusitemswizardselectionpage.h"
#include "ui_importcorpusitemswizardselectionpage.h"

ImportCorpusItemsWizardSelectionPage::ImportCorpusItemsWizardSelectionPage(
        QPointer<Corpus> corpus,
        QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &candidateRecordings,
        QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &candidateAnnotations,
        QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ImportCorpusItemsWizardSelectionPage),
    m_corpus(corpus), m_candidateRecordings(candidateRecordings), m_candidateAnnotations(candidateAnnotations),
    m_abort(false)
{
    ui->setupUi(this);
    connect(ui->commandSelectFolder, SIGNAL(clicked()), this, SLOT(selectFolder()));
    connect(ui->commandAbort, SIGNAL(clicked()), this, SLOT(abortProcess()));

    // Set path
    ui->editFolderName->setText(m_corpus->basePath());

    // Check lists for file formats
    m_modelFormatsRecording = new QStandardItemModel(this);
    m_modelFormatsAnnotation = new QStandardItemModel(this);
    m_modelFormatsRecording->setColumnCount(1);
    m_modelFormatsAnnotation->setColumnCount(1);
    QStandardItem *item;
    // Media formats
    item = new QStandardItem("Wave files (.wav)");  item->setData("wav");
    item->setCheckable(true);   item->setCheckState(Qt::Checked);
    m_modelFormatsRecording->setItem(0, 0, item);
    item = new QStandardItem("MP3 files (.mp3)");  item->setData("mp3");
    item->setCheckable(true);   item->setCheckState(Qt::Checked);
    m_modelFormatsRecording->setItem(1, 0, item);
    ui->treeviewRecordingsFormat->setModel(m_modelFormatsRecording);
    ui->treeviewRecordingsFormat->setHeaderHidden(true);
    // Annotation formats
    item = new QStandardItem("Praat textgrid files (.textgrid)");  item->setData("textgrid");
    item->setCheckable(true);   item->setCheckState(Qt::Checked);
    m_modelFormatsAnnotation->setItem(0, 0, item);
    item = new QStandardItem("TranscriberAG files (.trs)");  item->setData("trs");
    item->setCheckable(true);   item->setCheckState(Qt::Checked);
    m_modelFormatsAnnotation->setItem(1, 0, item);
    ui->treeviewAnnotationsFormat->setModel(m_modelFormatsAnnotation);
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
    m_abort = true;
}

bool ImportCorpusItemsWizardSelectionPage::validatePage ()
{
    ui->stackedWidget->setCurrentIndex(1);
    qDeleteAll(m_candidateRecordings);
    m_candidateRecordings.clear();
    qDeleteAll(m_candidateAnnotations);
    m_candidateAnnotations.clear();
    ui->texteditMessages->clear();
    m_abort = false;

    QDir dirinfo(ui->editFolderName->text());
    QStringList recordingFilters, annotationFilters;
    for (int i = 0; i < m_modelFormatsRecording->rowCount(); ++i) {
        QStandardItem *item = m_modelFormatsRecording->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            QString suffix = item->data().toString().toLower();
            recordingFilters << QString("*.") + suffix;
        }
    }
    for (int i = 0; i < m_modelFormatsAnnotation->rowCount(); ++i) {
        QStandardItem *item = m_modelFormatsAnnotation->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            QString suffix = item->data().toString();
            annotationFilters << QString("*.") + suffix;
        }
    }

    if (!recordingFilters.isEmpty()) {
        addFiles(dirinfo.canonicalPath(), recordingFilters, ui->checkBoxRecursive->isChecked(), true);
        if (m_abort) {
            ui->stackedWidget->setCurrentIndex(0);
            return false;
        }
    }
    if (!annotationFilters.isEmpty()) {
        addFiles(dirinfo.canonicalPath(), annotationFilters, ui->checkBoxRecursive->isChecked(), false);
        if (m_abort) {
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
    if (m_abort) return;
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
    if (m_abort) return false;
    bool isAnnotation = !isRecording;
    if (!m_modelFormatsRecording) return false;
    if (!m_modelFormatsAnnotation) return false;

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
    QString communicationID = communicationNameFromFilename(info);
    if (communicationID.isEmpty()) return false;
    if (isRecording) {
        CorpusRecording *rec = new CorpusRecording(baseFilename);
        rec->setName(baseFilename);
        rec->setFilename(info.canonicalFilePath());
        m_candidateRecordings.insert(QPair<QString, QString>(communicationID, baseFilename), rec);
        ui->texteditMessages->appendPlainText(QString("MEDIA RECORDING %1 >> Communication ID: %2, Recording ID: %3")
                                              .arg(info.canonicalFilePath()).arg(communicationID).arg(baseFilename));
        ui->texteditMessages->moveCursor(QTextCursor::End);
    } else if (isAnnotation) {
        CorpusAnnotation *annot = new CorpusAnnotation(communicationID); // baseFilename
        annot->setFilename(info.canonicalFilePath());
        // Speaker Policy
        if (ui->optionSpeakerSingle->isChecked()) {
            annot->setProperty("speakerPolicy", SpeakerPolicySingle);
            annot->setProperty("speakerPolicyData", annot->ID());
        }
        else if (ui->optionSpeakerTiers->isChecked()) {
            annot->setProperty("speakerPolicy", SpeakerPolicyTierNames);
            annot->setProperty("speakerPolicyData", "");
        }
        else if (ui->optionSpeakerIntervals->isChecked()) {
            annot->setProperty("speakerPolicy", SpeakerPolicyIntervals);
            annot->setProperty("speakerPolicyData", "speaker");
        }
        else if (ui->optionSpeakerPrimaryAndSecondary->isChecked()) {
            annot->setProperty("speakerPolicy", SpeakerPolicyPrimaryAndSecondary);
            annot->setProperty("speakerPolicyData", "ortho;loc2");
        }

        m_candidateAnnotations.insert(QPair<QString, QString>(communicationID, baseFilename), annot);
        ui->texteditMessages->appendPlainText(QString("ANNOTATION %1 >> Communication ID: %2, Annotation ID: %3")
                                              .arg(info.canonicalFilePath()).arg(communicationID).arg(baseFilename));
        ui->texteditMessages->moveCursor(QTextCursor::End);
    }
    return true;
}

// private
QString ImportCorpusItemsWizardSelectionPage::communicationNameFromFilename(const QFileInfo &fileInfo)
{
    QString filename = fileInfo.baseName();
    if (ui->optionSubfolderName->isChecked()) {
        QString path = fileInfo.canonicalFilePath();
        QStringList folders = path.split("/");
        if (folders.isEmpty() || folders.count() == 1) return filename;
        // canoncial file path = /abc/def/xyz/file.ext => we need the second-to-last item
        return folders.at(folders.count() - 2);
    }
    else if (ui->optionEntireFilename->isChecked()) {
        return filename;
    }
    else if (ui->optionRemoveFromEnd->isChecked()) {
        QStringList stringsToRemove = ui->editRemoveFromEnd->text().split(",");
        foreach (QString stringToRemove, stringsToRemove) {
            QString ret = filename;
            if (ret.endsWith(stringToRemove.trimmed())) ret.chop(stringToRemove.trimmed().length());
            return ret;
        }
        return filename;
    }
    else if (ui->optionTakeLeft->isChecked()) {
        return filename.mid(0, ui->spinBoxLeftChars->value());
    }
    else if (ui->optionRemoveLeft->isChecked()) {
        return filename.mid(ui->spinBoxLeftChars->value(), -1);
    }
    else if (ui->optionTakeRight->isChecked()) {
        return filename.right(ui->spinBoxRightChars->value());
    }
    else if (ui->optionRemoveRight->isChecked()) {
        if (filename.length() > ui->spinBoxRightChars->value())
            return filename.mid(0, filename.length() - ui->spinBoxRightChars->value());
        else
            return filename;
    }
    return QString();
}


