#ifndef ANNOTATIONSTRUCTUREEDITOR_H
#define ANNOTATIONSTRUCTUREEDITOR_H

#include <QWidget>
#include "pncore/corpus/CorpusObject.h"
#include "pngui/observers/CorpusObserver.h"

namespace Ui {
class AnnotationStructureEditor;
}

namespace Praaline {
namespace Core {
class AnnotationStructure;
}
}

struct AnnotationStructureEditorData;

class AnnotationStructureEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnnotationStructureEditor(QWidget *parent = 0);
    ~AnnotationStructureEditor();

private slots:
    void activeCorpusRepositoryChanged(const QString &repositoryID);

    void saveAnnotationStructure();
    void addAnnotationStructureLevel();
    void addAnnotationStructureAttribute();
    void removeAnnotationStructureItem();
    void renameAnnotationLevel(const QString &oldID, const QString &newID);
    void renameAnnotationAttribute(const QString &levelID, const QString &oldID, const QString &newID);

    void importAnnotationStructure();
    void exportAnnotationStructure();

    void cloneAnnotationStructureLevel();

private:
    Ui::AnnotationStructureEditor *ui;
    AnnotationStructureEditorData *d;

    void setupActions();
    void refreshAnnotationStructureTreeView(Praaline::Core::AnnotationStructure *structure);
};

#endif // ANNOTATIONSTRUCTUREEDITOR_H
