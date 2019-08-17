#ifndef IMPORTDATAPREVIEWWIDGET_H
#define IMPORTDATAPREVIEWWIDGET_H

#include <QWidget>

namespace Ui {
class ImportDataPreviewWidget;
}

struct ImportDataPreviewWidgetData;

class ImportDataPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImportDataPreviewWidget(QWidget *parent = nullptr);
    ~ImportDataPreviewWidget();

private slots:
    void selectFile();
    void importParametersChangedTextFile();
    void importParametersChangedExcel();

private:
    Ui::ImportDataPreviewWidget *ui;
    ImportDataPreviewWidgetData *d;

    void readTextFile();
    void readExcel();
    void preparePreview();
};

#endif // IMPORTDATAPREVIEWWIDGET_H
