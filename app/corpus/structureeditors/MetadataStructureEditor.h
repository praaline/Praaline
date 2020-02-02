#ifndef METADATASTRUCTUREEDITOR_H
#define METADATASTRUCTUREEDITOR_H

#include <QMainWindow>
#include "PraalineCore/Corpus/CorpusObject.h"
#include "pngui/observers/CorpusObserver.h"

namespace Ui {
class MetadataStructureEditor;
}

namespace Praaline {
namespace Core {
class MetadataStructure;
}
}

struct MetadataStructureEditorData;

class MetadataStructureEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit MetadataStructureEditor(QWidget *parent = nullptr);
    ~MetadataStructureEditor();

private slots:
    void activeCorpusRepositoryChanged(const QString &repositoryID);

    void saveMetadataStructure();
    void addMetadataStructureSection();
    void addMetadataStructureAttribute();
    void removeMetadataStructureItem();
    void renameMetadataAttribute(Praaline::Core::CorpusObject::Type type,
                                 const QString &oldID, const QString &newID);
    void importMetadataStructure();
    void exportMetadataStructure();

private:
    Ui::MetadataStructureEditor *ui;
    MetadataStructureEditorData *d;

    void setupActions();
    void refreshMetadataStructureTreeView(Praaline::Core::MetadataStructure *structure);
};

#endif // METADATASTRUCTUREEDITOR_H
