#ifndef IMPORTMETADATAWIZARD_H
#define IMPORTMETADATAWIZARD_H

#include <QWizard>
#include <QList>
#include <QMap>
#include <QStandardItemModel>
#include <QItemSelection>
#include <QLineEdit>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class ImportMetadataWizard;
}

struct ImportMetadataWizardData;

class ImportMetadataWizard : public QWizard
{
    Q_OBJECT

public:
    class ColumnCorrespondance {
    public:
        ColumnCorrespondance() : column(-1) {}
        ColumnCorrespondance(QString attributeID, QString displayName, QString datatype) :
            attributeID(attributeID), displayName(displayName), datatype(datatype), column(-1) {}
        QString attributeID;
        QString displayName;
        QString datatype;
        int column;
        QString formatString;
    };

    ImportMetadataWizard(const QString &filename, Corpus *corpus, QWidget *parent = 0);
    ~ImportMetadataWizard();

private:
    Ui::ImportMetadataWizard *ui;
    ImportMetadataWizardData *d;

    void readFile();
    void preparePreview();
    void guessCorrespondances();
    void updateColumnHeaders();
    void previewImport();
    void doImport();

    // virtual methods from QWizard
    bool validateCurrentPage();
    void initializePage(int id);

private slots:
    void objectTypeChanged();
    void fileParametersChanged();
    void previewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void correspondanceChanged();
};



#endif // IMPORTMETADATAWIZARD_H
