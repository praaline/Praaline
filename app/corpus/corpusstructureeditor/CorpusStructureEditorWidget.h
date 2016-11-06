#ifndef CORPUSSTRUCTUREWIDGET_H
#define CORPUSSTRUCTUREWIDGET_H

#include <QWidget>
#include "pncore/corpus/CorpusObject.h"
#include "pngui/observers/corpusobserver.h"

namespace Ui {
class CorpusStructureEditorWidget;
}

namespace Praaline {
namespace Core {
class MetadataStructure;
class AnnotationStructure;
}
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
    void renameMetadataAttribute(Praaline::Core::CorpusObject::Type type,
                                 const QString &oldID, const QString &newID);
    void importMetadataStructure();
    void exportMetadataStructure();

    void addAnnotationStructureLevel();
    void addAnnotationStructureAttribute();
    void removeAnnotationStructureItem();
    void renameAnnotationLevel(const QString &oldID, const QString &newID);
    void renameAnnotationAttribute(const QString &levelID, const QString &oldID, const QString &newID);

    void importAnnotationStructure();
    void exportAnnotationStructure();

    void cloneAnnotationStructureLevel();

private:
    Ui::CorpusStructureEditorWidget *ui;
    CorpusStructureEditorWidgetData *d;

    void setupActions();
    void refreshMetadataStructureTreeView(Praaline::Core::MetadataStructure *structure);
    void refreshAnnotationStructureTreeView(Praaline::Core::AnnotationStructure *structure);
};

#endif // CORPUSSTRUCTUREWIDGET_H
