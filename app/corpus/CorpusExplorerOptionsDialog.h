#ifndef CORPUSEXPLOREROPTIONSDIALOG_H
#define CORPUSEXPLOREROPTIONSDIALOG_H

#include <QDialog>
#include <QPointer>
#include <QStringList>
#include <QStandardItemModel>
#include "pncore/structure/metadatastructure.h"

namespace Ui {
class CorpusExplorerOptionsDialog;
}

struct CorpusExplorerOptionsDialogData;

class CorpusExplorerOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CorpusExplorerOptionsDialog(MetadataStructure *mstructure, QWidget *parent = 0);
    ~CorpusExplorerOptionsDialog();

    QStringList groupAttributesForCommunications() const;
    QStringList groupAttributesForSpeakers() const;
    QStringList displayAttributesForCommunications() const;
    QStringList displayAttributesForSpeakers() const;

private slots:
    void commandGroup();
    void commandUngroup();
    void commandGroupMoveUp();
    void commandGroupMoveDown();

private:
    Ui::CorpusExplorerOptionsDialog *ui;
    CorpusExplorerOptionsDialogData *d;

    QPointer<QStandardItemModel> createAttributeModel(CorpusObject::Type type, bool checkable);
};

#endif // CORPUSEXPLOREROPTIONSDIALOG_H
