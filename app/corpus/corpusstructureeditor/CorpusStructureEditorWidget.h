#ifndef CORPUSSTRUCTUREWIDGET_H
#define CORPUSSTRUCTUREWIDGET_H

#include <QWidget>
#include "pngui/observers/corpusobserver.h"

namespace Ui {
class CorpusStructureEditorWidget;
}

struct CorpusStructureEditorWidgetData;

class CorpusStructureEditorWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit CorpusStructureEditorWidget(QWidget *parent = 0);
    ~CorpusStructureEditorWidget();

private slots:
    void activeCorpusChanged(const QString &newActiveCorpusID);

    void addMetadataStructureSection();
    void addMetadataStructureAttribute();
    void removeMetadataStructureItem();
    void importMetadataStructure();
    void exportMetadataStructure();

    void addAnnotationStructureLevel();
    void addAnnotationStructureAttribute();
    void removeAnnotationStructureItem();
    void importAnnotationStructure();
    void exportAnnotationStructure();

    void duplicateAnnotationStructureLevel();

private:
    Ui::CorpusStructureEditorWidget *ui;
    CorpusStructureEditorWidgetData *d;

    void setupActions();
    void refreshMetadataStructureTreeView(MetadataStructure *structure);
    void refreshAnnotationStructureTreeView(AnnotationStructure *structure);
};

#endif // CORPUSSTRUCTUREWIDGET_H
