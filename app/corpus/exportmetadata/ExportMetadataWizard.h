#ifndef EXPORTMETADATAWIZARD_H
#define EXPORTMETADATAWIZARD_H

#include <QWizard>
#include "pncore/corpus/CorpusObject.h"
#include "pncore/corpus/CorpusObjectInfo.h"
using namespace Praaline::Core;

namespace Ui {
class ExportMetadataWizard;
}

class QStandardItemModel;

struct ExportMetadataWizardData;

class ExportMetadataWizard : public QWizard
{
    Q_OBJECT

public:
    explicit ExportMetadataWizard(QWidget *parent = nullptr);
    ~ExportMetadataWizard();

private slots:
    void selectFileFolder();
    void exportFormatChanged(int);
    void corpusRepositoryChanged(const QString &repositoryID);
    void corpusObjectClicked(const QModelIndex &index);

private:
    Ui::ExportMetadataWizard *ui;
    ExportMetadataWizardData *d;

    QStandardItemModel *createAttributeModel(CorpusObject::Type type, bool checkable);

    // Override methods from QWizard
    bool validateCurrentPage() override;

    void doExport();
    bool doExportText(CorpusObject::Type type, const QList<CorpusObjectInfo> &list,
                      const QList<QPair<QString, QString> > &attributes);
};

#endif // EXPORTMETADATAWIZARD_H
