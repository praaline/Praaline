#ifndef EXPORTVISUALISATIONDIALOG_H
#define EXPORTVISUALISATIONDIALOG_H

#include <QDialog>

namespace Ui {
class ExportVisualisationDialog;
}

struct ExportVisualisationDialogData;
class Document;
class PaneStack;
class ViewManager;

class ExportVisualisationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportVisualisationDialog(Document *document, PaneStack *paneStack, ViewManager *viewManager,
                                       QWidget *parent = 0);
    ~ExportVisualisationDialog();

private slots:
    void doExport();

private:
    Ui::ExportVisualisationDialog *ui;
    ExportVisualisationDialogData *d;
};

#endif // EXPORTVISUALISATIONDIALOG_H
