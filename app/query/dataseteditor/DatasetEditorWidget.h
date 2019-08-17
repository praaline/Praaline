#ifndef DATASETEDITORWIDGET_H
#define DATASETEDITORWIDGET_H

#include <QWidget>
#include "nodeeditor/DataModelRegistry.h"

struct DatasetEditorWidgetData;

class DatasetEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DatasetEditorWidget(QWidget *parent = nullptr);
    ~DatasetEditorWidget();

    static std::shared_ptr<DataModelRegistry> registerDataModels();
    static void setStyle();

signals:

public slots:

private:
    DatasetEditorWidgetData *d;
};

#endif // DATASETEDITORWIDGET_H
