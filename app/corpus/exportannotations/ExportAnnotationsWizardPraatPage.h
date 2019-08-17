#ifndef EXPORTANNOTATIONSWIZARDPRAATPAGE_H
#define EXPORTANNOTATIONSWIZARDPRAATPAGE_H

#include <QPointer>
#include <QWizardPage>

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
using namespace Praaline::Core;

namespace Ui {
class ExportAnnotationsWizardPraatPage;
}

struct ExportAnnotationsWizardPraatPageData;

class ExportAnnotationsWizardPraatPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ExportAnnotationsWizardPraatPage(QWidget *parent = nullptr);
    ~ExportAnnotationsWizardPraatPage();

    void setRepository(CorpusRepository *repository);
    void setExportPath(const QString &path);
    void doExport(const QList<CorpusObjectInfo> &annotations);

private slots:
    void annotationLevelAttributeSelectionChanged(const QString &levelID, const QString &attributeID, bool selected);
    void textgridStructureMoveUp();
    void textgridStructureMoveDown();
    void changedSpeakerPolicy();

private:
    Ui::ExportAnnotationsWizardPraatPage *ui;
    ExportAnnotationsWizardPraatPageData *d;
};

#endif // EXPORTANNOTATIONSWIZARDPRAATPAGE_H
