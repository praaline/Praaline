#include <QString>
#include <QFile>
#include <QTextStream>
#include <QClipboard>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QProgressDialog>

#include "GridViewFindDialog.h"
#include "grid/QAbstractFilterProxyModel.h"
#include "GridViewWidget.h"
#include "ui_GridViewWidget.h"

#include "xlsx/xlsxdocument.h"

struct GridViewWidgetData {
    GridViewWidgetData() {}

    QShortcut *shortcutCopy;
    GridViewFindDialog *findDialog;
};

GridViewWidget::GridViewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GridViewWidget),
    d(new GridViewWidgetData)
{
    ui->setupUi(this);

    QMenu* m = new QMenu(this);
    m->addAction(ui->singleViewAction);
    m->addAction(ui->horizontalSplitAction);
    ui->splitModeToolButton->setMenu(m);
    ui->splitModeToolButton->setDefaultAction(ui->singleViewAction);
    connect(ui->pinRowsToolButton, SIGNAL(clicked(bool)), this, SLOT(pinRowsToolButtonClicked()));
    connect(ui->singleViewAction, SIGNAL(triggered(bool)), this, SLOT(splitGridActionTriggered()));
    connect(ui->horizontalSplitAction, SIGNAL(triggered(bool)), this, SLOT(splitGridActionTriggered()));
    connect(ui->filterToolButton, SIGNAL(clicked(bool)), this, SLOT(toggleFilterRows()));

    connect(ui->clipboardCopyToolButton, SIGNAL(clicked(bool)), this, SLOT(clipboardCopySelection()));
    ui->clipboardCopyToolButton->setShortcut(QKeySequence("Ctrl+C"));
    connect(ui->clipboardPasteToolButton, SIGNAL(clicked(bool)), this, SLOT(clipboardPaste()));
    ui->clipboardPasteToolButton->setShortcut(QKeySequence("Ctrl+V"));
    connect(ui->findToolButton, SIGNAL(clicked(bool)), this, SLOT(findDialog()));
    ui->findToolButton->setShortcut(QKeySequence("Ctrl+F"));
    connect(ui->exportToolButton, SIGNAL(clicked(bool)), this, SLOT(exportToFile()));

    d->findDialog = new GridViewFindDialog(this);
    d->findDialog->setModal(false);
    connect(d->findDialog, SIGNAL(findNext()), this, SLOT(findNext()));

    ui->advancedTableView->setVerticalHeadersSize(20);

    // default result text
    connect(ui->advancedTableView->filterProxyModel(), SIGNAL(resultCountChanged(int,int)),
            this, SLOT(resultCountChanged(int,int)));
}

GridViewWidget::~GridViewWidget()
{
    delete ui;
    delete d;
}

QAdvancedTableView *GridViewWidget::tableView()
{
    return ui->advancedTableView;
}

void GridViewWidget::setResultsLabelText(const QString &text)
{
    ui->advancedTableViewResultLabel->setText(text);
}

void GridViewWidget::resultCountChanged(int filterRows, int unfilteredRows)
{
    if (unfilteredRows - filterRows > 0)
        setResultsLabelText(tr("Result: %1 of %2").arg(filterRows).arg(unfilteredRows));
    else
        setResultsLabelText(tr("Count: %1").arg(filterRows));
}

void GridViewWidget::exportToFile()
{
    QString filename;
    if (filename.isEmpty()) {
        QFileDialog::Options options;
        QString selectedFilter;
        filename = QFileDialog::getSaveFileName(this, tr("Export to tab-separated file"),
                                                "praaline_data.xlsx", tr("Excel Spreadsheet (*.xlsx);;Text File (*.txt);;All Files (*)"),
                                                &selectedFilter, options);
    }
    if (filename.isEmpty()) return;
    if (filename.endsWith(".xlsx"))
        exportToExcel(filename);
    else
        exportToTabSeparated(filename);
}

bool GridViewWidget::exportToTabSeparated(const QString &fileName)
{
    // Get model and verify it is ok
    QAbstractItemModel *model = ui->advancedTableView->model();
    if (!model) return false;
    // Open text file
    QFile fileOut(fileName);
    if (! fileOut.open(QFile::WriteOnly | QFile::Text)) return false;
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    // Write header
    for (int col = 0; col < model->columnCount(); ++col) {
        out << model->headerData(col, Qt::Horizontal).toString() << "\t";
    }
    out << "\n";
    // Create a process dialog while writing data
    int count = model->rowCount();
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, count);
    progressDialog.setWindowTitle(tr("Exporting..."));
    // Write data
    for (int row = 0; row < count; ++row) {
        if (progressDialog.wasCanceled()) break;
        progressDialog.setValue(row);
        progressDialog.setLabelText(tr("Row %1 of %2...").arg(row).arg(count));
        qApp->processEvents();

        for (int col = 0; col < model->columnCount(); ++col) {
            out << model->data(model->index(row, col)).toString();
            if (col != model->columnCount() - 1) out << "\t";
        }
        out << "\n";
    }
    // Finished
    fileOut.close();
    return true;
}

bool GridViewWidget::exportToExcel(const QString &fileName)
{
    // Get model and verify it is ok
    QAbstractItemModel *model = ui->advancedTableView->model();
    if (!model) return false;
    // Create an Excel document
    QXlsx::Document xlsx;
    // Note: rows and columns start from 1 for QXlsx
    // Write header
    QXlsx::Format format_header;
    format_header.setFontBold(true);
    format_header.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    for (int col = 0; col < model->columnCount(); ++col) {
        xlsx.write(1, col + 1, model->headerData(col, Qt::Horizontal), format_header);
    }
    // Create a process dialog while writing data
    int count = model->rowCount();
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, count);
    progressDialog.setWindowTitle(tr("Exporting..."));
    // Write data
    for (int row = 0; row < count; ++row) {
        if (progressDialog.wasCanceled()) break;
        progressDialog.setValue(row);
        progressDialog.setLabelText(tr("Row %1 of %2...").arg(row).arg(count));
        qApp->processEvents();
        for (int col = 0; col < model->columnCount(); ++col) {
            xlsx.write(row + 2, col + 1, model->data(model->index(row, col)));
        }
    }
    // Finished
    xlsx.saveAs(fileName);
    return true;
}

void GridViewWidget::clipboardCopySelection()
{
    QAbstractItemModel *abmodel = ui->advancedTableView->model();
    if (!abmodel) return;
    QItemSelectionModel * model = ui->advancedTableView->selectionModel();
    if (!model) return;
    QModelIndexList list = model->selectedIndexes();
    if(list.size() < 1) return;

    qSort(list);

    QString copy_table;
    QModelIndex previous = list.first();

    list.removeFirst();
    if (list.isEmpty()) return;

    for(int i = 0; i < list.size(); i++) {
        QVariant data = abmodel->data(previous);
        QString text = data.toString();

        QModelIndex index = list.at(i);
        copy_table.append(text);

        if(index.row() != previous.row())
            copy_table.append('\n');
        else
            copy_table.append('\t');
        previous = index;
    }

    copy_table.append(abmodel->data(list.last()).toString());
    copy_table.append('\n');

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(copy_table);
}

void GridViewWidget::clipboardPaste()
{
    QAbstractItemModel *abmodel = ui->advancedTableView->model();
    if (!abmodel) return;
    QItemSelectionModel *smodel = ui->advancedTableView->selectionModel();
    if (!smodel) return;
    QModelIndexList list = smodel->selectedIndexes();
    if(list.size() < 1) return;

    qSort(list);

    QString str = QApplication::clipboard()->text();
    QStringList rows = str.split('\n');
    int numRows = rows.count();
    if (numRows < 1) return;
    int numColumns = rows.first().count('\t') + 1;

    if (list.size() > 1) {
        int rowCount = list.last().row() - list.first().row();
        int columnCount = list.last().column() - list.first().column();
        if ((rowCount * columnCount != 1) && ((rowCount != numRows) || (columnCount != numColumns))) {
            QMessageBox::information(this, tr("Error"),
                                     tr("The information cannot be pasted because the copy "
                                        "and paste areas aren't the same size."));
            return;
        }
    }

    for (int i = 0; i < numRows; ++i) {
        QStringList columns = rows[i].split('\t');
        for (int j = 0; j < columns.count(); ++j) {
            int row = list.at(0).row() + i;
            int column = list.at(0).column() + j;
            abmodel->setData(abmodel->index(row, column), columns[j]);
        }
    }
}

void GridViewWidget::pinRowsToolButtonClicked()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (ui->advancedTableView->showFixedRows()){
        ui->advancedTableView->setShowFixedRows(false);
        ui->pinRowsToolButton->setIcon(QIcon(":/qaiv/pin/enabled"));
    } else {
        ui->advancedTableView->setShowFixedRows(true);
        ui->pinRowsToolButton->setIcon(QIcon(":/qaiv/pin/disabled"));
    }
    QApplication::restoreOverrideCursor();
}

void GridViewWidget::splitGridActionTriggered()
{
    if (sender() == ui->singleViewAction){
        ui->advancedTableView->splitView(false);
        ui->splitModeToolButton->setIcon(ui->singleViewAction->icon());
    } else if (sender() == ui->horizontalSplitAction){
        ui->advancedTableView->splitView(true);
        ui->splitModeToolButton->setIcon(ui->horizontalSplitAction->icon());
    }
}

void GridViewWidget::findDialog()
{
    d->findDialog->show();
    d->findDialog->raise();
    d->findDialog->activateWindow();
}

bool GridViewWidget::matches(const QVariant &data, const QString &findWhat,
                             bool matchWholeWordsOnly, bool matchCase, bool useRegularExpressions)
{
    bool found = false;
    if (useRegularExpressions) {
        QRegularExpression rx = QRegularExpression(findWhat);
        if (rx.match(data.toString()).hasMatch()) found = true;
    }
    else {
        if (matchWholeWordsOnly) {
            if (matchCase) {
                if (data.toString() == findWhat) found = true;
            } else {
                if (data.toString().toUpper() == findWhat.toUpper()) found = true;
            }
        } else {
            if (data.toString().contains(findWhat, (matchCase) ? Qt::CaseSensitive : Qt::CaseInsensitive))
                found = true;
        }
    }
    return found;
}

void GridViewWidget::findNext()
{
    QString findWhat = d->findDialog->findWhat();
    if (findWhat.isEmpty()) return;
    bool matchWholeWordsOnly = d->findDialog->matchWholeWordsOnly();
    bool matchCase = d->findDialog->matchCase();
    bool useRegularExpressions = d->findDialog->useRegularExpressions();

    QAbstractItemModel *abmodel = ui->advancedTableView->model();
    if (!abmodel) return;
    QItemSelectionModel *smodel = ui->advancedTableView->selectionModel();
    if (!smodel) return;

    int row = ui->advancedTableView->currentIndex().row();
    int column = ui->advancedTableView->currentIndex().column();

    if (d->findDialog->searchScope() == GridViewFindDialog::EntireTable) {
        column++;
        while (row < abmodel->rowCount()) {
            while (column < abmodel->columnCount()) {
                QVariant data = abmodel->data(abmodel->index(row, column));
                if (matches(data, findWhat, matchWholeWordsOnly, matchCase, useRegularExpressions)) {
                    ui->advancedTableView->setCurrentIndex(abmodel->index(row, column));
                    smodel->setCurrentIndex(abmodel->index(row, column), QItemSelectionModel::ClearAndSelect);
                    d->findDialog->hide();
                    return;
                }
                ++column;
            }
            column = 0;
            ++row;
        }
    }
    else if (d->findDialog->searchScope() == GridViewFindDialog::Rows) {
        row++;
        while (row < abmodel->rowCount()) {
            QVariant data = abmodel->data(abmodel->index(row, column));
            if (matches(data, findWhat, matchWholeWordsOnly, matchCase, useRegularExpressions)) {
                ui->advancedTableView->setCurrentIndex(abmodel->index(row, column));
                smodel->setCurrentIndex(abmodel->index(row, column), QItemSelectionModel::ClearAndSelect);
                d->findDialog->hide();
                return;
            }
            ++row;
        }
    }
    else if (d->findDialog->searchScope() == GridViewFindDialog::Columns) {
        column++;
        while (column < abmodel->columnCount()) {
            QVariant data = abmodel->data(abmodel->index(row, column));
            if (matches(data, findWhat, matchWholeWordsOnly, matchCase, useRegularExpressions)) {
                ui->advancedTableView->setCurrentIndex(abmodel->index(row, column));
                smodel->setCurrentIndex(abmodel->index(row, column), QItemSelectionModel::ClearAndSelect);
                d->findDialog->hide();
                return;
            }
            ++column;
        }
    }
}

void GridViewWidget::toggleFilterRows()
{
    ui->advancedTableView->setShowFilter(!ui->advancedTableView->showFilter());
}
