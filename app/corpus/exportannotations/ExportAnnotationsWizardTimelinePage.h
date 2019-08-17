#ifndef EXPORTANNOTATIONSWIZARDTIMELINEPAGE_H
#define EXPORTANNOTATIONSWIZARDTIMELINEPAGE_H

#include <QPointer>
#include <QWizardPage>

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
using namespace Praaline::Core;

namespace Ui {
class ExportAnnotationsWizardTimelinePage;
}

struct ExportAnnotationsWizardTimelinePageData;

class ExportAnnotationsWizardTimelinePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ExportAnnotationsWizardTimelinePage(QWidget *parent = nullptr);
    ~ExportAnnotationsWizardTimelinePage();

    void setExportPath(const QString &path);
    void setRepository(CorpusRepository *repository);
    void doExport(const QList<CorpusObjectInfo> &annotations);

private slots:
    void annotationLevelAttributeSelectionChanged(const QString &levelID, const QString &attributeID, bool selected);
    void timelineStructureAdd();
    void timelineStructureDelete();
    void timelineStructureMoveUp();
    void timelineStructureMoveDown();

private:
    Ui::ExportAnnotationsWizardTimelinePage *ui;
    ExportAnnotationsWizardTimelinePageData *d;

    QString interpretFilenameTemplate(const QString &filenameTemplate, const CorpusObjectInfo &info);
};

#endif // EXPORTANNOTATIONSWIZARDTIMELINEPAGE_H
