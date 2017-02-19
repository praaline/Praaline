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
    explicit ImportDataPreviewWidget(QWidget *parent = 0);
    ~ImportDataPreviewWidget();

private:
    Ui::ImportDataPreviewWidget *ui;
    ImportDataPreviewWidgetData *d;
};

#endif // IMPORTDATAPREVIEWWIDGET_H
