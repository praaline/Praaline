#ifndef EXPORTMETADATAWIZARD_H
#define EXPORTMETADATAWIZARD_H

#include <QWizard>
#include "pncore/corpus/CorpusObject.h"
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
    explicit ExportMetadataWizard(QWidget *parent = 0);
    ~ExportMetadataWizard();

private slots:
    void exportFormatChanged(int);
    void corpusRepositoryChanged(QString);

private:
    Ui::ExportMetadataWizard *ui;
    ExportMetadataWizardData *d;

    QStandardItemModel *createAttributeModel(CorpusObject::Type type, bool checkable);
};

#endif // EXPORTMETADATAWIZARD_H
