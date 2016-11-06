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
#include "pngui/model/corpus/CorpusCommunicationTableModel.h"
#include "pngui/model/corpus/CorpusSpeakerTableModel.h"
#include "pngui/model/corpus/CorpusRecordingTableModel.h"
#include "pngui/model/corpus/CorpusAnnotationTableModel.h"
#include "pngui/model/corpus/CorpusParticipationTableModel.h"


ImportMetadataWizard::ImportMetadataWizard(const QString &filename, Corpus *corpus, QWidget *parent) :
    QWizard(parent), ui(new Ui::ImportMetadataWizard),
    m_corpus(corpus), m_filename(filename), m_columnCount(0), m_previewModel(0)
{
    ui->setupUi(this);
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
    m_encoding = "UTF-8";
    m_delimiter = "\t";
    m_textQualifier = "";
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
}

// private slot
void ImportMetadataWizard::objectTypeChanged()
{
    if (ui->optionCommunications->isChecked()) m_corpusObjectType = CorpusObject::Type_Communication;
    else if (ui->optionSpeakers->isChecked()) m_corpusObjectType = CorpusObject::Type_Speaker;
    else if (ui->optionRecordings->isChecked()) m_corpusObjectType = CorpusObject::Type_Recording;
    else if (ui->optionAnnotations->isChecked()) m_corpusObjectType = CorpusObject::Type_Annotation;
    else if (ui->optionParticipations->isChecked()) m_corpusObjectType = CorpusObject::Type_Participation;

    m_ColumnCorrespondances.clear();
    ui->comboAttribute->clear();
    QPointer<MetadataStructure> mstr = m_corpus->metadataStructure();
    if (!mstr) return;
    QString mandatory = "You must link the following attributes to a column before being able to import corpus items: ";
    ui->comboAttribute->addItem("None (ignored)", "");
    if (ui->optionCommunications->isChecked()) {
        m_ColumnCorrespondances.insert("ID", ColumnCorrespondance("ID", "Communication ID", "text"));
        m_ColumnCorrespondances.insert("name", ColumnCorrespondance("name", "Communication Name", "text"));
        ui->comboAttribute->addItem("Communication ID", "ID");
        ui->comboAttribute->addItem("Communication Name", "name");
        ui->labelMandatoryAttributes->setText(mandatory + "<b>Communication ID</b>");
    }
    else if (ui->optionSpeakers->isChecked()) {
        m_ColumnCorrespondances.insert("ID", ColumnCorrespondance("ID", "Speaker ID", "text"));
        m_ColumnCorrespondances.insert("name", ColumnCorrespondance("name", "Speaker Name", "text"));
        ui->comboAttribute->addItem("Speaker ID", "ID");
        ui->comboAttribute->addItem("Speaker Name", "name");
        ui->labelMandatoryAttributes->setText(mandatory + "<b>Speaker ID</b>");
    }
    else if (ui->optionRecordings->isChecked()) {
        m_ColumnCorrespondances.insert("ID", ColumnCorrespondance("ID", "Recording ID", "text"));
        m_ColumnCorrespondances.insert("name", ColumnCorrespondance("name", "Recording Name", "text"));
        m_ColumnCorrespondances.insert("communicationID", ColumnCorrespondance("communicationID", "Communication ID", "text"));
        m_ColumnCorrespondances.insert("filename", ColumnCorrespondance("filename", "Filename", "text"));
        ui->comboAttribute->addItem("Recording ID", "ID");
        ui->comboAttribute->addItem("Recording Name", "name");
        ui->comboAttribute->addItem("Communication ID", "communicationID");
        ui->comboAttribute->addItem("Filename", "filename");
        ui->labelMandatoryAttributes->setText(mandatory + "<b>Recording ID, Communication ID</b>");
    }
    else if (ui->optionAnnotations->isChecked()) {
        m_ColumnCorrespondances.insert("ID", ColumnCorrespondance("ID", "Annotation ID", "text"));
        m_ColumnCorrespondances.insert("name", ColumnCorrespondance("name", "Annotation Name", "text"));
        m_ColumnCorrespondances.insert("communicationID", ColumnCorrespondance("communicationID", "Communication ID", "text"));
        ui->comboAttribute->addItem("Annotation ID", "ID");
        ui->comboAttribute->addItem("Annotation Name", "name");
        ui->comboAttribute->addItem("Communication ID", "communicationID");
        ui->labelMandatoryAttributes->setText(mandatory + "<b>Annotation ID, Communication ID</b>");
    }
    else if (ui->optionParticipations->isChecked()) {
        m_ColumnCorrespondances.insert("communicationID", ColumnCorrespondance("communicationID", "Communication ID", "text"));
        m_ColumnCorrespondances.insert("speakerID", ColumnCorrespondance("speakerID", "Speaker ID", "text"));
        m_ColumnCorrespondances.insert("role", ColumnCorrespondance("role", "Role", "text"));
        ui->comboAttribute->addItem("Communication ID", "communicationID");
        ui->comboAttribute->addItem("Speaker ID", "speakerID");
        ui->comboAttribute->addItem("Role", "role");
        ui->labelMandatoryAttributes->setText(mandatory + "<b>Communication ID, Speaker ID</b>");
    }
    foreach (QPointer<MetadataStructureAttribute> mattr, mstr->attributes(m_corpusObjectType)) {
        if (!mattr) continue;
        m_ColumnCorrespondances.insert(mattr->ID(), ColumnCorrespondance(mattr->ID(), mattr->name(), mattr->datatypeString()));
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
    if (selectedDelimiter == "Tab") m_delimiter = "\t";
    else if (selectedDelimiter == "Comma (,)") m_delimiter = ",";
    else if (selectedDelimiter == "Semicolon (;))") m_delimiter = ";";
    else if (selectedDelimiter == "Colon (:)") m_delimiter = ":";
    else if (selectedDelimiter == "Double colon (::)") m_delimiter = "::";
    // Text qualifier
    if (selectedTextQualifier == "None") m_textQualifier = "";
    else if (selectedTextQualifier == "Single quote (')") m_textQualifier = "'";
    else if (selectedTextQualifier == "Double quote (\")") m_textQualifier = "\"";
    // Encoding
    if (selectedEncoding.contains("\t"))
        m_encoding = selectedEncoding.section("\t", 1, 1);
    else
        m_encoding = selectedEncoding;
    // Reread file
    readFile();
}

// private slot
void ImportMetadataWizard::previewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    if (selected.indexes().isEmpty()) return;
    int j = selected.indexes().first().column();
    foreach (ColumnCorrespondance correspondance, m_ColumnCorrespondances) {
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
    if (!m_previewModel) return;
    QModelIndexList selection = ui->tableViewPreviewColumns->selectionModel()->selectedColumns();
    if (selection.isEmpty()) return;
    int column = selection.first().column();
    // reset this column
    foreach (QString attributeID, m_ColumnCorrespondances.keys())
        if (m_ColumnCorrespondances[attributeID].column == column)
            m_ColumnCorrespondances[attributeID].column = -1;
    m_previewModel->setHorizontalHeaderItem(column, new QStandardItem(""));
    // handle cases where the user wants the attribute ignored
    if (ui->comboAttribute->currentText() == "None (ignored)") return;
    // associate this column
    QString attributeID = ui->comboAttribute->currentData().toString();
    if (!m_ColumnCorrespondances.contains(attributeID)) return;
    m_ColumnCorrespondances[attributeID].column = column;
    if (ui->optionFormatText->isChecked()) {
        m_ColumnCorrespondances[attributeID].datatype = "text";
        m_ColumnCorrespondances[attributeID].formatString = "";
    }
    else if (ui->optionFormatNumber->isChecked()) {
        m_ColumnCorrespondances[attributeID].datatype = "number";
        m_ColumnCorrespondances[attributeID].formatString = "";
    }
    else if (ui->optionFormatDate->isChecked()) {
        m_ColumnCorrespondances[attributeID].datatype = "datatime";
        m_ColumnCorrespondances[attributeID].formatString = ui->comboDateFormat->currentText();
    }
    // update headers (all of them, to ensure all column changes are visible)
    updateColumnHeaders();
}

void ImportMetadataWizard::readFile()
{
    m_preview.clear();
    QString line;
    QFile file(m_filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QTextStream stream(&file);
    stream.setCodec(m_encoding.toLatin1().constData());
    int columnCount = 0;
    m_fileLineCount = 0;
    do {
        line = stream.readLine();
        if (m_fileLineCount <= 100) {
            QList<QString> fields = line.split(m_delimiter);
            if (columnCount < fields.count()) columnCount = fields.count();
            m_preview << fields;
        }
        m_fileLineCount++;
    } while (!stream.atEnd());
    file.close();
    m_columnCount = columnCount;
    // reset correspondances
    foreach (ColumnCorrespondance correspondance, m_ColumnCorrespondances) {
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
    if (m_previewModel) delete m_previewModel;
    m_previewModel = new QStandardItemModel(this);
    m_previewModel->setColumnCount(m_columnCount);
    m_previewModel->setRowCount((hasHeader) ? m_preview.count() - 1 : m_preview.count());
    for (int i = 0; i < m_preview.count(); ++i) {
        if (hasHeader && i == 0) continue;
        int j = 0;
        int model_i = ((hasHeader) ? i - 1 : i);
        foreach(QString field, m_preview.at(i)) {
            m_previewModel->setItem(model_i, j, new QStandardItem(field));
            j++;
        }
    }
    // link model to two preview table views
    ui->tableViewPreviewIntro->setModel(m_previewModel);
    ui->tableViewPreviewColumns->setModel(m_previewModel);
    connect(ui->tableViewPreviewColumns->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(previewSelectionChanged(QItemSelection,QItemSelection)));
    // next step
    guessCorrespondances();
}

void ImportMetadataWizard::guessCorrespondances()
{
    if (!ui->checkHeaderLine->isChecked()) return;
    if (m_preview.count() < 1) return;
    QList<QString> headers = m_preview.at(0);
    for (int j = 0; j < headers.count(); ++j) {
        if (m_ColumnCorrespondances.contains(headers[j])) {
            m_ColumnCorrespondances[headers[j]].column = j;
        }
    }
    updateColumnHeaders();
}

void ImportMetadataWizard::updateColumnHeaders()
{
    if (!m_previewModel) return;
    // empty header labels
    QStringList headers;
    for (int j = 0; j < m_columnCount; ++j) headers << "";
    m_previewModel->setHorizontalHeaderLabels(headers);
    // update header labels
    foreach (ColumnCorrespondance correspondence, m_ColumnCorrespondances) {
        if (correspondence.column != -1)
            m_previewModel->setHorizontalHeaderItem(correspondence.column, new QStandardItem(correspondence.displayName));
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
        if (m_corpusObjectType == CorpusObject::Type_Communication) {
            if (m_ColumnCorrespondances["ID"].column == -1) {
                QMessageBox::warning(this, "Error", mandatory + "<b>Communication ID</b>.", QMessageBox::Ok);
                return false;
            }
        }
        else if (m_corpusObjectType == CorpusObject::Type_Speaker) {
            if (m_ColumnCorrespondances["ID"].column == -1) {
                QMessageBox::warning(this, "Error", mandatory + "<b>Speaker ID</b>.", QMessageBox::Ok);
                return false;
            }
        }
        else if (m_corpusObjectType == CorpusObject::Type_Recording) {
            if ((m_ColumnCorrespondances["ID"].column == -1) || (m_ColumnCorrespondances["communicationID"].column == -1)) {
                QMessageBox::warning(this, "Error", mandatory + "<b>Recording ID, Communication ID</b>.", QMessageBox::Ok);
                return false;
            }
        }
        else if (m_corpusObjectType == CorpusObject::Type_Annotation) {
            if ((m_ColumnCorrespondances["ID"].column == -1) || (m_ColumnCorrespondances["communicationID"].column == -1)) {
                QMessageBox::warning(this, "Error", mandatory + "<b>Annotation ID, Communication ID</b>.", QMessageBox::Ok);
                return false;
            }
        }
        else if (m_corpusObjectType == CorpusObject::Type_Participation) {
            if ((m_ColumnCorrespondances["communicationID"].column == -1) || (m_ColumnCorrespondances["speakerID"].column == -1)) {
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
    if (!m_corpus) return;
    MetadataStructure *mstr = m_corpus->metadataStructure();
    QStandardItemModel *model = new QStandardItemModel();
    QString line;
    QFile file(m_filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QTextStream stream(&file);
    stream.setCodec(m_encoding.toLatin1().constData());
    bool firstLine = true;
    int i = 0;
    QFont font;
    font.setItalic(true);
    ui->progressBarImport->setValue(0);
    // List of attributes to show on the preview
    QStringList attributeList, headerList;
    if (m_corpusObjectType == CorpusObject::Type_Communication) {
        attributeList << "ID" << "name";
        headerList << "Communication ID" << "Name";
    }
    else if (m_corpusObjectType == CorpusObject::Type_Speaker) {
        attributeList << "ID" << "name";
        headerList << "Speaker ID" << "Name";
    }
    else if (m_corpusObjectType == CorpusObject::Type_Recording) {
        attributeList << "ID" << "communicationID" << "name";
        headerList << "Recording ID" << "Communication ID" << "Name";
    }
    else if (m_corpusObjectType == CorpusObject::Type_Annotation) {
        attributeList << "ID" << "communicationID" << "name";
        headerList << "Annotation ID" << "Communication ID" << "Name";
    }
    else if (m_corpusObjectType == CorpusObject::Type_Participation) {
        attributeList << "communicationID" << "speakerID" << "role";
        headerList << "Communication ID" << "Speaker ID" << "Role";
    }
    foreach (QPointer<MetadataStructureAttribute> attribute, mstr->attributes(m_corpusObjectType)) {
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
        if (m_corpusObjectType == CorpusObject::Type_Communication) {
            communicationID = line.section(m_delimiter, m_ColumnCorrespondances["ID"].column, m_ColumnCorrespondances["ID"].column);
            existing = m_corpus->communication(communicationID);
        }
        else if (m_corpusObjectType == CorpusObject::Type_Speaker) {
            speakerID = line.section(m_delimiter, m_ColumnCorrespondances["ID"].column, m_ColumnCorrespondances["ID"].column);
            existing = m_corpus->speaker(speakerID);
        }
        else if (m_corpusObjectType == CorpusObject::Type_Recording) {
            recordingID = line.section(m_delimiter, m_ColumnCorrespondances["ID"].column, m_ColumnCorrespondances["ID"].column);
            communicationID = line.section(m_delimiter, m_ColumnCorrespondances["communicationID"].column, m_ColumnCorrespondances["communicationID"].column);
            if (m_corpus->communication(communicationID)) existing = m_corpus->communication(communicationID)->recording(recordingID);
        }
        else if (m_corpusObjectType == CorpusObject::Type_Annotation) {
            annotationID = line.section(m_delimiter, m_ColumnCorrespondances["ID"].column, m_ColumnCorrespondances["ID"].column);
            communicationID = line.section(m_delimiter, m_ColumnCorrespondances["communicationID"].column, m_ColumnCorrespondances["communicationID"].column);
            if (m_corpus->communication(communicationID)) existing = m_corpus->communication(communicationID)->annotation(annotationID);
        }
        else if (m_corpusObjectType == CorpusObject::Type_Participation) {
            communicationID = line.section(m_delimiter, m_ColumnCorrespondances["communicationID"].column, m_ColumnCorrespondances["communicationID"].column);
            speakerID = line.section(m_delimiter, m_ColumnCorrespondances["speakerID"].column, m_ColumnCorrespondances["speakerID"].column);
            existing = m_corpus->participation(communicationID, speakerID);
        }
        int j = 0;
        foreach (QString attributeID, attributeList) {
            if (!m_ColumnCorrespondances.contains(attributeID)) continue;
            ColumnCorrespondance correspondance = m_ColumnCorrespondances.value(attributeID);
            QStandardItem *item;
            if (correspondance.column == -1) {
                if (existing)
                    item = new QStandardItem(existing->property(attributeID).toString());
                else
                    item = new QStandardItem("");
                item->setFont(font);
            }
            else {
                QString field = line.section(m_delimiter, correspondance.column, correspondance.column);
                if (mstr->attribute(m_corpusObjectType, attributeID)) {
                    if (mstr->attribute(m_corpusObjectType, attributeID)->datatype().base() == DataType::Integer)
                        item = new QStandardItem(QString::number(field.toInt()));
                    else if (mstr->attribute(m_corpusObjectType, attributeID)->datatype().base() == DataType::Double)
                        item = new QStandardItem(QString::number(field.toDouble()));
                    else if (mstr->attribute(m_corpusObjectType, attributeID)->datatype().base() == DataType::DateTime)
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
        ui->progressBarImport->setValue((i * 100) / m_fileLineCount);
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
    if (!m_corpus) return;
    MetadataStructure *mstr = m_corpus->metadataStructure();
    QString line;
    QFile file(m_filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QTextStream stream(&file);
    stream.setCodec(m_encoding.toLatin1().constData());
    bool firstLine = true;
    do {
        line = stream.readLine();
        if (firstLine && ui->checkHeaderLine->isChecked()) {
            firstLine = false;
            continue; // skip header line
        }
        QString communicationID, speakerID, recordingID, annotationID;
        CorpusObject *item = 0;
        if (m_corpusObjectType == CorpusObject::Type_Communication) {
            communicationID = line.section(m_delimiter, m_ColumnCorrespondances["ID"].column, m_ColumnCorrespondances["ID"].column);
            if (ui->optionModeUpdate->isChecked())
                item = m_corpus->communication(communicationID);
            if (!item) {
                m_corpus->addCommunication(new CorpusCommunication(communicationID));
                item = m_corpus->communication(communicationID);
            }
        }
        else if (m_corpusObjectType == CorpusObject::Type_Speaker) {
            speakerID = line.section(m_delimiter, m_ColumnCorrespondances["ID"].column, m_ColumnCorrespondances["ID"].column);
            if (ui->optionModeUpdate->isChecked())
                item = m_corpus->speaker(speakerID);
            if (!item) {
                m_corpus->addSpeaker(new CorpusSpeaker(speakerID));
                item = m_corpus->speaker(speakerID);
            }
        }
        else if (m_corpusObjectType == CorpusObject::Type_Recording) {
            recordingID = line.section(m_delimiter, m_ColumnCorrespondances["ID"].column, m_ColumnCorrespondances["ID"].column);
            communicationID = line.section(m_delimiter, m_ColumnCorrespondances["communicationID"].column, m_ColumnCorrespondances["communicationID"].column);
            QPointer<CorpusCommunication> com = m_corpus->communication(communicationID);
            if (!com) continue; // skip this item when a corresponding communication does not exist
            if (ui->optionModeUpdate->isChecked())
                item = com->recording(recordingID);
            if (!item) {
                com->addRecording(new CorpusRecording(recordingID));
                item = com->recording(recordingID);
            }
        }
        else if (m_corpusObjectType == CorpusObject::Type_Annotation) {
            annotationID = line.section(m_delimiter, m_ColumnCorrespondances["ID"].column, m_ColumnCorrespondances["ID"].column);
            communicationID = line.section(m_delimiter, m_ColumnCorrespondances["communicationID"].column, m_ColumnCorrespondances["communicationID"].column);
            QPointer<CorpusCommunication> com = m_corpus->communication(communicationID);
            if (!com) continue; // skip this item when a corresponding communication does not exist
            if (ui->optionModeUpdate->isChecked())
                item = com->annotation(annotationID);
            if (!item) {
                com->addAnnotation(new CorpusAnnotation(annotationID));
                item = com->annotation(annotationID);
            }
        }
        else if (m_corpusObjectType == CorpusObject::Type_Participation) {
            communicationID = line.section(m_delimiter, m_ColumnCorrespondances["communicationID"].column, m_ColumnCorrespondances["communicationID"].column);
            speakerID = line.section(m_delimiter, m_ColumnCorrespondances["speakerID"].column, m_ColumnCorrespondances["speakerID"].column);
            qDebug() << m_ColumnCorrespondances["speakerID"].column << speakerID;
            QPointer<CorpusCommunication> com = m_corpus->communication(communicationID);
            QPointer<CorpusSpeaker> spk = m_corpus->speaker(speakerID);
            if (!com || !spk) continue;
            if (ui->optionModeUpdate->isChecked())
                item = m_corpus->participation(communicationID, speakerID);
            if (!item) {
                item = m_corpus->addParticipation(communicationID, speakerID);
            }
        }
        if (!item) continue; // this should never happen
        foreach (ColumnCorrespondance correspondance, m_ColumnCorrespondances) {
            if (correspondance.column == -1)
                continue; // this attribute is not linked to a column, skip
            QString field = line.section(m_delimiter, correspondance.column, correspondance.column);
            if (mstr->attribute(m_corpusObjectType, correspondance.attributeID)) {
                if  (mstr->attribute(m_corpusObjectType, correspondance.attributeID)->datatype().base() == DataType::Integer)
                    item->setProperty(correspondance.attributeID, field.toInt());
                else if (mstr->attribute(m_corpusObjectType, correspondance.attributeID)->datatype().base() == DataType::Double)
                    item->setProperty(correspondance.attributeID, field.toDouble());
                else if (mstr->attribute(m_corpusObjectType, correspondance.attributeID)->datatype().base() == DataType::DateTime)
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

