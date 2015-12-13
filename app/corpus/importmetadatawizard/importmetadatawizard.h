#ifndef IMPORTMETADATAWIZARD_H
#define IMPORTMETADATAWIZARD_H

#include <QWizard>
#include <QList>
#include <QMap>
#include <QStandardItemModel>
#include <QItemSelection>
#include <QLineEdit>
#include "pncore/corpus/corpus.h"

namespace Ui {
class ImportMetadataWizard;
}

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

    Corpus *m_corpus;
    CorpusObject::Type m_corpusObjectType;
    QString m_filename;
    QString m_encoding;
    QString m_delimiter;
    QString m_textQualifier;
    QList<QList<QString> > m_preview;
    int m_columnCount;
    int m_fileLineCount;
    QStandardItemModel *m_previewModel;
    QMap<QString, ColumnCorrespondance> m_ColumnCorrespondances;

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
