#ifndef EXPORTMETADATAWIZARD_H
#define EXPORTMETADATAWIZARD_H

#include <QWizard>
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class ExportMetadataWizard;
}

struct ExportMetadataWizardData;

class ExportMetadataWizard : public QWizard
{
    Q_OBJECT

public:
    explicit ExportMetadataWizard(Corpus *corpus, QWidget *parent = 0);
    ~ExportMetadataWizard();

private:
    Ui::ExportMetadataWizard *ui;
    ExportMetadataWizardData *d;
};

#endif // EXPORTMETADATAWIZARD_H
