#include <QString>
#include <QList>
#include <QPointer>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardItemModel>

#include "xlsx/xlsxdocument.h"

#include "ImportDataPreviewWidget.h"
#include "ui_ImportDataPreviewWidget.h"

struct ImportDataPreviewWidgetData {
    QString filename;
    QString encoding;
    QString delimiter;
    QString textQualifier;
    QString excelSheetName;
    bool hasHeader;
    int skipRows;
    QList<QList<QString> > preview;
    int columnCount;
    int fileLineCount;
    QPointer<QStandardItemModel> previewModel;
};

ImportDataPreviewWidget::ImportDataPreviewWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::ImportDataPreviewWidget), d(new ImportDataPreviewWidgetData)
{
    ui->setupUi(this);
    // Select file command
    connect(ui->commandSelectFile, &QAbstractButton::clicked, this, &ImportDataPreviewWidget::selectFile);
    // Preview table
    ui->tableViewPreview->verticalHeader()->setDefaultSectionSize(20);
    ui->tableViewPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewPreview->setSelectionBehavior(QAbstractItemView::SelectColumns);
    ui->tableViewPreview->setSelectionMode(QAbstractItemView::SingleSelection);
    // Populate combo boxes
    ui->comboBoxTextFileDelimiter->addItem("Tab", "\t");
    ui->comboBoxTextFileDelimiter->addItem("Comma (,)", ",");
    ui->comboBoxTextFileDelimiter->addItem("Semicolon (;)", ";");
    ui->comboBoxTextFileDelimiter->addItem("Colon (:)", ":");
    ui->comboBoxTextFileDelimiter->addItem("Double colon (::)", "::");
    ui->comboBoxTextFileTextQualifier->addItem("None", "");
    ui->comboBoxTextFileTextQualifier->addItem("Single quote (')", "'");
    ui->comboBoxTextFileTextQualifier->addItem("Double quote (\")", "\"");
    QStringList encodings;
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
    ui->comboBoxTextFileEncoding->addItems(encodings);
    // Sensible defaults (set before connecting signals to slots to avoid side-effects)
    d->hasHeader = true;
    d->skipRows = 0;
    d->delimiter = "\t";
    d->textQualifier = "";
    d->encoding = "UTF-8";
    ui->spinBoxTextFileSkipRows->setValue(0);
    ui->spinBoxExcelSkipRows->setValue(0);
    ui->optionTextFileHasHeader->setChecked(true);
    ui->optionExcelHasHeader->setChecked(true);
    // Connect signals to respond to import parameter changes (update preview)
    // Text file
    connect(ui->comboBoxTextFileDelimiter, qOverload<int>(&QComboBox::currentIndexChanged), this, &ImportDataPreviewWidget::importParametersChangedTextFile);
    connect(ui->comboBoxTextFileTextQualifier, qOverload<int>(&QComboBox::currentIndexChanged), this, &ImportDataPreviewWidget::importParametersChangedTextFile);
    connect(ui->comboBoxTextFileEncoding, qOverload<int>(&QComboBox::currentIndexChanged), this, &ImportDataPreviewWidget::importParametersChangedTextFile);
    connect(ui->optionTextFileHasHeader, &QCheckBox::stateChanged, this, &ImportDataPreviewWidget::importParametersChangedTextFile);
    connect(ui->spinBoxTextFileSkipRows, qOverload<int>(&QSpinBox::valueChanged), this, &ImportDataPreviewWidget::importParametersChangedTextFile);
    // Excel
    connect(ui->comboBoxExcelSheet, qOverload<int>(&QComboBox::currentIndexChanged), this, &ImportDataPreviewWidget::importParametersChangedExcel);
    connect(ui->optionExcelHasHeader, &QCheckBox::stateChanged, this, &ImportDataPreviewWidget::importParametersChangedExcel);
    connect(ui->spinBoxExcelSkipRows, qOverload<int>(&QSpinBox::valueChanged), this, &ImportDataPreviewWidget::importParametersChangedExcel);
    // Start with text file properties
    ui->stackedWidget->setCurrentIndex(0);
}

ImportDataPreviewWidget::~ImportDataPreviewWidget()
{
    delete ui;
    delete d;
}

void ImportDataPreviewWidget::selectFile()
{
    QString filename;
    QFileDialog::Options options;
    QString selectedFilter;
    filename = QFileDialog::getOpenFileName(this, tr("Select file to import"), "",
                                            tr("Excel Spreadsheet (*.xlsx);;Text File (*.txt);;All Files (*)"),
                                            &selectedFilter, options);
    if (filename.isEmpty()) return;
    d->filename = filename;
    ui->editFilename->setText(filename);
    if (filename.endsWith(".xlsx")) {
        ui->stackedWidget->setCurrentIndex(1);
        QXlsx::Document xlsx(d->filename);
        ui->comboBoxExcelSheet->clear();
        ui->comboBoxExcelSheet->addItems(xlsx.sheetNames());
        importParametersChangedExcel();
    } else {
        ui->stackedWidget->setCurrentIndex(0);
        importParametersChangedTextFile();
    }
}

void ImportDataPreviewWidget::importParametersChangedTextFile()
{
    // Delimiter, text qualifier
    d->delimiter = ui->comboBoxTextFileDelimiter->currentData().toString();
    d->textQualifier = ui->comboBoxTextFileTextQualifier->currentData().toString();
    // Encoding
    QString encoding = ui->comboBoxTextFileEncoding->currentText();
    if (encoding.contains("\t"))
        d->encoding = encoding.section("\t", 1, 1);
    else
        d->encoding = encoding;
    // Header line, skip rows
    d->hasHeader = ui->optionTextFileHasHeader->isChecked();
    d->skipRows = ui->spinBoxTextFileSkipRows->value();
    // Reread file
    readTextFile();
}


void ImportDataPreviewWidget::importParametersChangedExcel()
{
    d->excelSheetName = ui->comboBoxExcelSheet->currentText();
    d->hasHeader = ui->optionExcelHasHeader->isChecked();
    d->skipRows = ui->spinBoxExcelSkipRows->value();
    readExcel();
}

void ImportDataPreviewWidget::readTextFile()
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
    preparePreview();
}

void ImportDataPreviewWidget::readExcel()
{
    d->preview.clear();
    QXlsx::Document xlsx(d->filename);
    if (!xlsx.selectSheet(d->excelSheetName)) return;
    QXlsx::Worksheet *sheet = xlsx.currentWorksheet();
    if (!sheet) return;
    // Get calculated results instead of formulas in cells
    sheet->setFormulasVisible(false);
    // Get row and column count for preview
    int rowCount = sheet->dimension().rowCount();
    if (rowCount > 50) rowCount = 50;
    int columnCount = sheet->dimension().columnCount();
    // Loop through preview rows
    for (int row = d->skipRows + 1; row <= rowCount; ++row) {
        QList<QString> fields;
        for (int column = 1; column <= columnCount; ++column) {
            fields << sheet->read(row, column).toString();
        }
        d->preview << fields;
    }
    d->columnCount = columnCount;
    preparePreview();
}

void ImportDataPreviewWidget::preparePreview()
{
    if (d->previewModel) delete d->previewModel;
    d->previewModel = new QStandardItemModel(this);
    d->previewModel->setColumnCount(d->columnCount);
    d->previewModel->setRowCount((d->hasHeader) ? d->preview.count() - 1 : d->preview.count());
    for (int i = 0; i < d->preview.count(); ++i) {
        int j = 0;
        // In case the file has a header, and this is the first line, set column labels
        if (d->hasHeader && i == 0) {
            d->previewModel->setHorizontalHeaderLabels(d->preview.at(i));
        }
        // Otherwise, process row
        int model_i = ((d->hasHeader) ? i - 1 : i);
        foreach(QString field, d->preview.at(i)) {
            d->previewModel->setItem(model_i, j, new QStandardItem(field));
            j++;
        }
    }
    // Show preview
    ui->tableViewPreview->setModel(d->previewModel);
}
