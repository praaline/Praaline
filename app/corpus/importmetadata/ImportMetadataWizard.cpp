#include <QList>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include "ImportMetadataWizard.h"
#include "ui_ImportMetadataWizard.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/structure/MetadataStructure.h"

#include "pngui/model/corpus/CorpusCommunicationTableModel.h"
#include "pngui/model/corpus/CorpusSpeakerTableModel.h"
#include "pngui/model/corpus/CorpusRecordingTableModel.h"
#include "pngui/model/corpus/CorpusAnnotationTableModel.h"
#include "pngui/model/corpus/CorpusParticipationTableModel.h"

struct ImportMetadataWizardData {
    ImportMetadataWizardData() : columnCount(0), fileLineCount(0) {}

    QPointer<Corpus> corpus;
    CorpusObject::Type corpusObjectType;
    QString filename;
    QString encoding;
    QString delimiter;
    QString textQualifier;
    QList<QList<QString> > preview;
    int columnCount;
    int fileLineCount;
    QPointer<QStandardItemModel> previewModel;
    QMap<QString, ImportMetadataWizard::ColumnCorrespondance> columnCorrespondances;
};

ImportMetadataWizard::ImportMetadataWizard(const QString &filename, Corpus *corpus, QWidget *parent) :
    QWizard(parent), ui(new Ui::ImportMetadataWizard), d(new ImportMetadataWizardData)
{
    ui->setupUi(this);

    d->corpus = corpus;
    d->filename = filename;

    ui->tableViewPreviewIntro->verticalHeader()->setDefaultSectionSize(20);
    ui->tableViewPreviewIntro->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewPreviewIntro->setSelectionBehavior(QAbstractItemView::SelectColumns);
    ui->tableViewPreviewIntro->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewPreviewColumns->verticalHeader()->setDefaultSectionSize(20);
    ui->tableViewPreviewColumns->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewPreviewColumns->setSelectionBehavior(QAbstractItemView::SelectColumns);
    ui->tableViewPreviewColumns->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewResults->verticalHeader()->setDefaultSectionSize(20);
    ui->tableViewResults->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Populate combo boxes
    QStringList delimiters, textQualifiers, dateFormats, encodings;
    delimiters << "Tab" << "Comma (,)" << "Semicolon (;)" << "Colon (:)" << "Double colon (::)";
    ui->comboDelimiter->addItems(delimiters);
    textQualifiers << "None" << "Single quote (')" << "Double quote (\")";
    ui->comboTextQualifier->addItems(textQualifiers);
    encodings << "(Unicode)\tUTF-8" << "(Unicode)\tUTF-16" << "(Unicode)\tUTF-16BE" << "(Unicode)\tUTF-16LE" << "(Unicode)\tUTF-32" <<
                 "(Unicode)\tUTF-32BE" << "(Unicode)\tUTF-32LE" <<
                 "ISO 8859-1" << "ISO 8859-2" << "ISO 8859-3" << "ISO 8859-4" << "ISO 8859-5" << "ISO 8859-6" <<
                 "ISO 8859-7" << "ISO 8859-8" << "ISO 8859-8" << "ISO 8859-10" << "ISO 8859-13" << "ISO 8859-14" <<
                 "ISO 8859-15" << "ISO 8859-16" << "Apple Roman" << "Big5" << "Big5-HKSCS" << "CP949" << "EUC-JP" << "EUC-KR" <<
                 "GB18030-0" << "IBM 850" << "IBM 866" << "IBM 874" << "ISO 2022-JP" << "Iscii-Bng" << "Iscii-Dev" << "Iscii-Gjr" <<
                 "Iscii-Knd" << "Iscii-Mlm" << "Iscii-Ori" << "Iscii-Pnj" << "Iscii-Tlg" << "Iscii-Tml" << "JIS X 0201" << "JIS X 0208" <<
                 "KOI8-R" << "KOI8-U" << "MuleLao-1" << "ROMAN8" << "Shift-JIS" << "TIS-620" << "TSCII" << "Windows-1250" << "Windows-1251" <<
                 "Windows-1252" << "Windows-1253" << "Windows-1254" << "Windows-1255" << "Windows-1256" << "Windows-1257" << "Windows-1258" <<
                 "WINSAMI2";
    ui->comboEncoding->addItems(encodings);
    dateFormats << "d/M/yyyy" << "M/d/yyyy" << "d/M/yy" << "M/d/yy";
    ui->comboDateFormat->addItems(dateFormats);
    // Sensible defaults (set before connecting signals to slots to avoid side-effects)
    d->encoding = "UTF-8";
    d->delimiter = "\t";
    d->textQualifier = "";
    ui->optionCommunications->setChecked(true);
    ui->checkHeaderLine->setChecked(true);
    ui->optionFormatText->setChecked(true);
    // Create initial attribute/column correspondance list (for Communications objects)
    objectTypeChanged();
    // Signals and slots
    connect(ui->optionCommunications, SIGNAL(toggled(bool)), this, SLOT(objectTypeChanged()));
    connect(ui->optionSpeakers, SIGNAL(toggled(bool)), this, SLOT(objectTypeChanged()));
    connect(ui->optionRecordings, SIGNAL(toggled(bool)), this, SLOT(objectTypeChanged()));
    connect(ui->optionAnnotations, SIGNAL(toggled(bool)), this, SLOT(objectTypeChanged()));
    connect(ui->optionParticipations, SIGNAL(toggled(bool)), this, SLOT(objectTypeChanged()));
    connect(ui->comboDelimiter, SIGNAL(currentIndexChanged(int)), this, SLOT(fileParametersChanged()));
    connect(ui->comboTextQualifier, SIGNAL(currentIndexChanged(int)), this, SLOT(fileParametersChanged()));
    connect(ui->comboEncoding, SIGNAL(currentIndexChanged(int)), this, SLOT(fileParametersChanged()));
    connect(ui->checkHeaderLine, SIGNAL(stateChanged(int)), this, SLOT(fileParametersChanged()));
    connect(ui->comboAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(correspondanceChanged()));
    connect(ui->optionFormatText, SIGNAL(toggled(bool)), this, SLOT(correspondanceChanged()));
    connect(ui->optionFormatNumber, SIGNAL(toggled(bool)), this, SLOT(correspondanceChanged()));
    connect(ui->optionFormatDate, SIGNAL(toggled(bool)), this, SLOT(correspondanceChanged()));
    connect(ui->comboDateFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(correspondanceChanged()));
    // Read file for preview
    readFile();
}

ImportMetadataWizard::~ImportMetadataWizard()
{
    delete ui;
    delete d;
}

// private slot
void ImportMetadataWizard::objectTypeChanged()
{
    if (ui->optionCommunications->isChecked()) d->corpusObjectType = CorpusObject::Type_Communication;
    else if (ui->optionSpeakers->isChecked()) d->corpusObjectType = CorpusObject::Type_Speaker;
    else if (ui->optionRecordings->isChecked()) d->corpusObjectType = CorpusObject::Type_Recording;
    else if (ui->optionAnnotations->isChecked()) d->corpusObjectType = CorpusObject::Type_Annotation;
    else if (ui->optionParticipations->isChecked()) d->corpusObjectType = CorpusObject::Type_Participation;

    d->columnCorrespondances.clear();
    ui->comboAttribute->clear();
    QPointer<MetadataStructure> mstr = d->corpus->repository()->metadataStructure();
    if (!mstr) return;
    QString mandatory = "You must link the following attributes to a column before being able to import corpus items: ";
    ui->comboAttribute->addItem("None (ignored)", "");
    if (ui->optionCommunications->isChecked()) {
        d->columnCorrespondances.insert("ID", ColumnCorrespondance("ID", "Communication ID", "text"));
        d->columnCorrespondances.insert("name", ColumnCorrespondance("name", "Communication Name", "text"));
        ui->comboAttribute->addItem("Communication ID", "ID");
        ui->comboAttribute->addItem("Communication Name", "name");
        ui->labelMandatoryAttributes->setText(mandatory + "<b>Communication ID</b>");
    }
    else if (ui->optionSpeakers->isChecked()) {
        d->columnCorrespondances.insert("ID", ColumnCorrespondance("ID", "Speaker ID", "text"));
        d->columnCorrespondances.insert("name", ColumnCorrespondance("name", "Speaker Name", "text"));
        ui->comboAttribute->addItem("Speaker ID", "ID");
        ui->comboAttribute->addItem("Speaker Name", "name");
        ui->labelMandatoryAttributes->setText(mandatory + "<b>Speaker ID</b>");
    }
    else if (ui->optionRecordings->isChecked()) {
        d->columnCorrespondances.insert("ID", ColumnCorrespondance("ID", "Recording ID", "text"));
        d->columnCorrespondances.insert("name", ColumnCorrespondance("name", "Recording Name", "text"));
        d->columnCorrespondances.insert("communicationID", ColumnCorrespondance("communicationID", "Communication ID", "text"));
        d->columnCorrespondances.insert("filename", ColumnCorrespondance("filename", "Filename", "text"));
        ui->comboAttribute->addItem("Recording ID", "ID");
        ui->comboAttribute->addItem("Recording Name", "name");
        ui->comboAttribute->addItem("Communication ID", "communicationID");
        ui->comboAttribute->addItem("Filename", "filename");
        ui->labelMandatoryAttributes->setText(mandatory + "<b>Recording ID, Communication ID</b>");
    }
    else if (ui->optionAnnotations->isChecked()) {
        d->columnCorrespondances.insert("ID", ColumnCorrespondance("ID", "Annotation ID", "text"));
        d->columnCorrespondances.insert("name", ColumnCorrespondance("name", "Annotation Name", "text"));
        d->columnCorrespondances.insert("communicationID", ColumnCorrespondance("communicationID", "Communication ID", "text"));
        ui->comboAttribute->addItem("Annotation ID", "ID");
        ui->comboAttribute->addItem("Annotation Name", "name");
        ui->comboAttribute->addItem("Communication ID", "communicationID");
        ui->labelMandatoryAttributes->setText(mandatory + "<b>Annotation ID, Communication ID</b>");
    }
    else if (ui->optionParticipations->isChecked()) {
        d->columnCorrespondances.insert("communicationID", ColumnCorrespondance("communicationID", "Communication ID", "text"));
        d->columnCorrespondances.insert("speakerID", ColumnCorrespondance("speakerID", "Speaker ID", "text"));
        d->columnCorrespondances.insert("role", ColumnCorrespondance("role", "Role", "text"));
        ui->comboAttribute->addItem("Communication ID", "communicationID");
        ui->comboAttribute->addItem("Speaker ID", "speakerID");
        ui->comboAttribute->addItem("Role", "role");
        ui->labelMandatoryAttributes->setText(mandatory + "<b>Communication ID, Speaker ID</b>");
    }
    foreach (QPointer<MetadataStructureAttribute> mattr, mstr->attributes(d->corpusObjectType)) {
        if (!mattr) continue;
        d->columnCorrespondances.insert(mattr->ID(), ColumnCorrespondance(mattr->ID(), mattr->name(), mattr->datatypeString()));
        ui->comboAttribute->addItem(mattr->name(), mattr->ID());
    }
    guessCorrespondances();
}

// private slot
void ImportMetadataWizard::fileParametersChanged()
{
    QString selectedDelimiter = ui->comboDelimiter->currentText();
    QString selectedTextQualifier = ui->comboTextQualifier->currentText();
    QString selectedEncoding = ui->comboEncoding->currentText();
    // Delimiter
    if (selectedDelimiter == "Tab") d->delimiter = "\t";
    else if (selectedDelimiter == "Comma (,)") d->delimiter = ",";
    else if (selectedDelimiter == "Semicolon (;))") d->delimiter = ";";
    else if (selectedDelimiter == "Colon (:)") d->delimiter = ":";
    else if (selectedDelimiter == "Double colon (::)") d->delimiter = "::";
    // Text qualifier
    if (selectedTextQualifier == "None") d->textQualifier = "";
    else if (selectedTextQualifier == "Single quote (')") d->textQualifier = "'";
    else if (selectedTextQualifier == "Double quote (\")") d->textQualifier = "\"";
    // Encoding
    if (selectedEncoding.contains("\t"))
        d->encoding = selectedEncoding.section("\t", 1, 1);
    else
        d->encoding = selectedEncoding;
    // Reread file
    readFile();
}

// private slot
void ImportMetadataWizard::previewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    if (selected.indexes().isEmpty()) return;
    int j = selected.indexes().first().column();
    foreach (ColumnCorrespondance correspondance, d->columnCorrespondances) {
        if (correspondance.column == j) {
            ui->comboAttribute->setCurrentText(correspondance.displayName);
            if (correspondance.datatype == "text") ui->optionFormatText->setChecked(true);
            else if (correspondance.datatype == "number") ui->optionFormatNumber->setChecked(true);
            else if (correspondance.datatype == "datetime") {
                ui->optionFormatDate->setChecked(true);
                ui->comboDateFormat->setCurrentText(correspondance.formatString);
            }
            return;
        }
    }
}

// private slot
void ImportMetadataWizard::correspondanceChanged()
{
    if (!d->previewModel) return;
    QModelIndexList selection = ui->tableViewPreviewColumns->selectionModel()->selectedColumns();
    if (selection.isEmpty()) return;
    int column = selection.first().column();
    // reset this column
    foreach (QString attributeID, d->columnCorrespondances.keys())
        if (d->columnCorrespondances[attributeID].column == column)
            d->columnCorrespondances[attributeID].column = -1;
    d->previewModel->setHorizontalHeaderItem(column, new QStandardItem(""));
    // handle cases where the user wants the attribute ignored
    if (ui->comboAttribute->currentText() == "None (ignored)") return;
    // associate this column
    QString attributeID = ui->comboAttribute->currentData().toString();
    if (!d->columnCorrespondances.contains(attributeID)) return;
    d->columnCorrespondances[attributeID].column = column;
    if (ui->optionFormatText->isChecked()) {
        d->columnCorrespondances[attributeID].datatype = "text";
        d->columnCorrespondances[attributeID].formatString = "";
    }
    else if (ui->optionFormatNumber->isChecked()) {
        d->columnCorrespondances[attributeID].datatype = "number";
        d->columnCorrespondances[attributeID].formatString = "";
    }
    else if (ui->optionFormatDate->isChecked()) {
        d->columnCorrespondances[attributeID].datatype = "datatime";
        d->columnCorrespondances[attributeID].formatString = ui->comboDateFormat->currentText();
    }
    // update headers (all of them, to ensure all column changes are visible)
    updateColumnHeaders();
}

void ImportMetadataWizard::readFile()
{
    d->preview.clear();
    QString line;
    QFile file(d->filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QTextStream stream(&file);
    stream.setCodec(d->encoding.toLatin1().constData());
    int columnCount = 0;
    d->fileLineCount = 0;
    do {
        line = stream.readLine();
        if (d->fileLineCount <= 100) {
            QList<QString> fields = line.split(d->delimiter);
            if (columnCount < fields.count()) columnCount = fields.count();
            d->preview << fields;
        }
        d->fileLineCount++;
    } while (!stream.atEnd());
    file.close();
    d->columnCount = columnCount;
    // reset correspondances
    foreach (ColumnCorrespondance correspondance, d->columnCorrespondances) {
        correspondance.column = -1;
        correspondance.datatype = "text";
        correspondance.formatString = "";
    }
    // next step
    preparePreview();
}

void ImportMetadataWizard::preparePreview()
{
    bool hasHeader = ui->checkHeaderLine->isChecked();
    if (d->previewModel) delete d->previewModel;
    d->previewModel = new QStandardItemModel(this);
    d->previewModel->setColumnCount(d->columnCount);
    d->previewModel->setRowCount((hasHeader) ? d->preview.count() - 1 : d->preview.count());
    for (int i = 0; i < d->preview.count(); ++i) {
        if (hasHeader && i == 0) continue;
        int j = 0;
        int model_i = ((hasHeader) ? i - 1 : i);
        foreach(QString field, d->preview.at(i)) {
            d->previewModel->setItem(model_i, j, new QStandardItem(field));
            j++;
        }
    }
    // link model to two preview table views
    ui->tableViewPreviewIntro->setModel(d->previewModel);
    ui->tableViewPreviewColumns->setModel(d->previewModel);
    connect(ui->tableViewPreviewColumns->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(previewSelectionChanged(QItemSelection,QItemSelection)));
    // next step
    guessCorrespondances();
}

void ImportMetadataWizard::guessCorrespondances()
{
    if (!ui->checkHeaderLine->isChecked()) return;
    if (d->preview.count() < 1) return;
    QList<QString> headers = d->preview.at(0);
    for (int j = 0; j < headers.count(); ++j) {
        if (d->columnCorrespondances.contains(headers[j])) {
            d->columnCorrespondances[headers[j]].column = j;
        }
    }
    updateColumnHeaders();
}

void ImportMetadataWizard::updateColumnHeaders()
{
    if (!d->previewModel) return;
    // empty header labels
    QStringList headers;
    for (int j = 0; j < d->columnCount; ++j) headers << "";
    d->previewModel->setHorizontalHeaderLabels(headers);
    // update header labels
    foreach (ColumnCorrespondance correspondence, d->columnCorrespondances) {
        if (correspondence.column != -1)
            d->previewModel->setHorizontalHeaderItem(correspondence.column, new QStandardItem(correspondence.displayName));
    }
}

void ImportMetadataWizard::initializePage(int id)
{
    if (id == 2)
        previewImport();
    return QWizard::initializePage(id);
}

bool ImportMetadataWizard::validateCurrentPage()
{
    QString mandatory = "Before you can import corpus items and their metadata, you must indicate which column contains the following data: ";
    if (currentId() == 1) {
        if (d->corpusObjectType == CorpusObject::Type_Communication) {
            if (d->columnCorrespondances["ID"].column == -1) {
                QMessageBox::warning(this, "Error", mandatory + "<b>Communication ID</b>.", QMessageBox::Ok);
                return false;
            }
        }
        else if (d->corpusObjectType == CorpusObject::Type_Speaker) {
            if (d->columnCorrespondances["ID"].column == -1) {
                QMessageBox::warning(this, "Error", mandatory + "<b>Speaker ID</b>.", QMessageBox::Ok);
                return false;
            }
        }
        else if (d->corpusObjectType == CorpusObject::Type_Recording) {
            if ((d->columnCorrespondances["ID"].column == -1) || (d->columnCorrespondances["communicationID"].column == -1)) {
                QMessageBox::warning(this, "Error", mandatory + "<b>Recording ID, Communication ID</b>.", QMessageBox::Ok);
                return false;
            }
        }
        else if (d->corpusObjectType == CorpusObject::Type_Annotation) {
            if ((d->columnCorrespondances["ID"].column == -1) || (d->columnCorrespondances["communicationID"].column == -1)) {
                QMessageBox::warning(this, "Error", mandatory + "<b>Annotation ID, Communication ID</b>.", QMessageBox::Ok);
                return false;
            }
        }
        else if (d->corpusObjectType == CorpusObject::Type_Participation) {
            if ((d->columnCorrespondances["communicationID"].column == -1) || (d->columnCorrespondances["speakerID"].column == -1)) {
                QMessageBox::warning(this, "Error", mandatory + "<b>Communication ID, Speaker ID</b>.", QMessageBox::Ok);
                return false;
            }
        }
    }
    else if (currentId() == 2) {
        doImport();
    }
    return true;
}

void ImportMetadataWizard::previewImport()
{
    if (!d->corpus) return;
    MetadataStructure *mstr = d->corpus->repository()->metadataStructure();
    QStandardItemModel *model = new QStandardItemModel();
    QString line;
    QFile file(d->filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QTextStream stream(&file);
    stream.setCodec(d->encoding.toLatin1().constData());
    bool firstLine = true;
    int i = 0;
    QFont font;
    font.setItalic(true);
    ui->progressBarImport->setValue(0);
    // List of attributes to show on the preview
    QStringList attributeList, headerList;
    if (d->corpusObjectType == CorpusObject::Type_Communication) {
        attributeList << "ID" << "name";
        headerList << "Communication ID" << "Name";
    }
    else if (d->corpusObjectType == CorpusObject::Type_Speaker) {
        attributeList << "ID" << "name";
        headerList << "Speaker ID" << "Name";
    }
    else if (d->corpusObjectType == CorpusObject::Type_Recording) {
        attributeList << "ID" << "communicationID" << "name";
        headerList << "Recording ID" << "Communication ID" << "Name";
    }
    else if (d->corpusObjectType == CorpusObject::Type_Annotation) {
        attributeList << "ID" << "communicationID" << "name";
        headerList << "Annotation ID" << "Communication ID" << "Name";
    }
    else if (d->corpusObjectType == CorpusObject::Type_Participation) {
        attributeList << "communicationID" << "speakerID" << "role";
        headerList << "Communication ID" << "Speaker ID" << "Role";
    }
    foreach (QPointer<MetadataStructureAttribute> attribute, mstr->attributes(d->corpusObjectType)) {
        if (attribute) attributeList << attribute->ID();
        headerList << attribute->name();
    }
    // Process file
    do {
        line = stream.readLine();
        if (firstLine && ui->checkHeaderLine->isChecked()) {
            firstLine = false;
            continue; // skip header line
        }
        CorpusObject *existing;
        QString communicationID, speakerID, recordingID, annotationID;
        if (d->corpusObjectType == CorpusObject::Type_Communication) {
            communicationID = line.section(d->delimiter, d->columnCorrespondances["ID"].column, d->columnCorrespondances["ID"].column);
            existing = d->corpus->communication(communicationID);
        }
        else if (d->corpusObjectType == CorpusObject::Type_Speaker) {
            speakerID = line.section(d->delimiter, d->columnCorrespondances["ID"].column, d->columnCorrespondances["ID"].column);
            existing = d->corpus->speaker(speakerID);
        }
        else if (d->corpusObjectType == CorpusObject::Type_Recording) {
            recordingID = line.section(d->delimiter, d->columnCorrespondances["ID"].column, d->columnCorrespondances["ID"].column);
            communicationID = line.section(d->delimiter, d->columnCorrespondances["communicationID"].column, d->columnCorrespondances["communicationID"].column);
            if (d->corpus->communication(communicationID)) existing = d->corpus->communication(communicationID)->recording(recordingID);
        }
        else if (d->corpusObjectType == CorpusObject::Type_Annotation) {
            annotationID = line.section(d->delimiter, d->columnCorrespondances["ID"].column, d->columnCorrespondances["ID"].column);
            communicationID = line.section(d->delimiter, d->columnCorrespondances["communicationID"].column, d->columnCorrespondances["communicationID"].column);
            if (d->corpus->communication(communicationID)) existing = d->corpus->communication(communicationID)->annotation(annotationID);
        }
        else if (d->corpusObjectType == CorpusObject::Type_Participation) {
            communicationID = line.section(d->delimiter, d->columnCorrespondances["communicationID"].column, d->columnCorrespondances["communicationID"].column);
            speakerID = line.section(d->delimiter, d->columnCorrespondances["speakerID"].column, d->columnCorrespondances["speakerID"].column);
            existing = d->corpus->participation(communicationID, speakerID);
        }
        int j = 0;
        foreach (QString attributeID, attributeList) {
            if (!d->columnCorrespondances.contains(attributeID)) continue;
            ColumnCorrespondance correspondance = d->columnCorrespondances.value(attributeID);
            QStandardItem *item;
            if (correspondance.column == -1) {
                if (existing)
                    item = new QStandardItem(existing->property(attributeID).toString());
                else
                    item = new QStandardItem("");
                item->setFont(font);
            }
            else {
                QString field = line.section(d->delimiter, correspondance.column, correspondance.column);
                if (mstr->attribute(d->corpusObjectType, attributeID)) {
                    if (mstr->attribute(d->corpusObjectType, attributeID)->datatype().base() == DataType::Integer)
                        item = new QStandardItem(QString::number(field.toInt()));
                    else if (mstr->attribute(d->corpusObjectType, attributeID)->datatype().base() == DataType::Double)
                        item = new QStandardItem(QString::number(field.toDouble()));
                    else if (mstr->attribute(d->corpusObjectType, attributeID)->datatype().base() == DataType::DateTime)
                        item = new QStandardItem(QDate::fromString(field, correspondance.formatString).toString());
                    else
                        item = new QStandardItem(field);
                }
                else
                    item = new QStandardItem(field);
            }
            model->setItem(i, j, item);
            j++;
        }
        i++;
        QApplication::processEvents();
        ui->progressBarImport->setValue((i * 100) / d->fileLineCount);
    } while (!stream.atEnd());
    file.close();
    ui->progressBarImport->setValue(100);
    model->setColumnCount(attributeList.count());
    model->setRowCount(i);
    model->setHorizontalHeaderLabels(headerList);
    ui->tableViewResults->setModel(model);
}

void ImportMetadataWizard::doImport()
{
    if (!d->corpus) return;
    MetadataStructure *mstr = d->corpus->repository()->metadataStructure();
    QString line;
    QFile file(d->filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QTextStream stream(&file);
    stream.setCodec(d->encoding.toLatin1().constData());
    bool firstLine = true;
    do {
        line = stream.readLine();
        if (firstLine && ui->checkHeaderLine->isChecked()) {
            firstLine = false;
            continue; // skip header line
        }
        QString communicationID, speakerID, recordingID, annotationID;
        CorpusObject *item = 0;
        if (d->corpusObjectType == CorpusObject::Type_Communication) {
            communicationID = line.section(d->delimiter, d->columnCorrespondances["ID"].column, d->columnCorrespondances["ID"].column);
            if (ui->optionModeUpdate->isChecked())
                item = d->corpus->communication(communicationID);
            if (!item) {
                d->corpus->addCommunication(new CorpusCommunication(communicationID));
                item = d->corpus->communication(communicationID);
            }
        }
        else if (d->corpusObjectType == CorpusObject::Type_Speaker) {
            speakerID = line.section(d->delimiter, d->columnCorrespondances["ID"].column, d->columnCorrespondances["ID"].column);
            if (ui->optionModeUpdate->isChecked())
                item = d->corpus->speaker(speakerID);
            if (!item) {
                d->corpus->addSpeaker(new CorpusSpeaker(speakerID));
                item = d->corpus->speaker(speakerID);
            }
        }
        else if (d->corpusObjectType == CorpusObject::Type_Recording) {
            recordingID = line.section(d->delimiter, d->columnCorrespondances["ID"].column, d->columnCorrespondances["ID"].column);
            communicationID = line.section(d->delimiter, d->columnCorrespondances["communicationID"].column, d->columnCorrespondances["communicationID"].column);
            QPointer<CorpusCommunication> com = d->corpus->communication(communicationID);
            if (!com) continue; // skip this item when a corresponding communication does not exist
            if (ui->optionModeUpdate->isChecked())
                item = com->recording(recordingID);
            if (!item) {
                com->addRecording(new CorpusRecording(recordingID));
                item = com->recording(recordingID);
            }
        }
        else if (d->corpusObjectType == CorpusObject::Type_Annotation) {
            annotationID = line.section(d->delimiter, d->columnCorrespondances["ID"].column, d->columnCorrespondances["ID"].column);
            communicationID = line.section(d->delimiter, d->columnCorrespondances["communicationID"].column, d->columnCorrespondances["communicationID"].column);
            QPointer<CorpusCommunication> com = d->corpus->communication(communicationID);
            if (!com) continue; // skip this item when a corresponding communication does not exist
            if (ui->optionModeUpdate->isChecked())
                item = com->annotation(annotationID);
            if (!item) {
                com->addAnnotation(new CorpusAnnotation(annotationID));
                item = com->annotation(annotationID);
            }
        }
        else if (d->corpusObjectType == CorpusObject::Type_Participation) {
            communicationID = line.section(d->delimiter, d->columnCorrespondances["communicationID"].column, d->columnCorrespondances["communicationID"].column);
            speakerID = line.section(d->delimiter, d->columnCorrespondances["speakerID"].column, d->columnCorrespondances["speakerID"].column);
            qDebug() << d->columnCorrespondances["speakerID"].column << speakerID;
            QPointer<CorpusCommunication> com = d->corpus->communication(communicationID);
            QPointer<CorpusSpeaker> spk = d->corpus->speaker(speakerID);
            if (!com || !spk) continue;
            if (ui->optionModeUpdate->isChecked())
                item = d->corpus->participation(communicationID, speakerID);
            if (!item) {
                item = d->corpus->addParticipation(communicationID, speakerID);
            }
        }
        if (!item) continue; // this should never happen
        foreach (ColumnCorrespondance correspondance, d->columnCorrespondances) {
            if (correspondance.column == -1)
                continue; // this attribute is not linked to a column, skip
            QString field = line.section(d->delimiter, correspondance.column, correspondance.column);
            if (mstr->attribute(d->corpusObjectType, correspondance.attributeID)) {
                if  (mstr->attribute(d->corpusObjectType, correspondance.attributeID)->datatype().base() == DataType::Integer)
                    item->setProperty(correspondance.attributeID, field.toInt());
                else if (mstr->attribute(d->corpusObjectType, correspondance.attributeID)->datatype().base() == DataType::Double)
                    item->setProperty(correspondance.attributeID, field.toDouble());
                else if (mstr->attribute(d->corpusObjectType, correspondance.attributeID)->datatype().base() == DataType::DateTime)
                    item->setProperty(correspondance.attributeID, QDate::fromString(field, correspondance.formatString));
                else
                    item->setProperty(correspondance.attributeID, field);
            }
            else
                item->setProperty(correspondance.attributeID, field);
        }
        QApplication::processEvents();
    } while (!stream.atEnd());
    file.close();
}

