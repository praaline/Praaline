#ifndef SELECTIONDIALOG_H
#define SELECTIONDIALOG_H

#include <QObject>
#include <QDialog>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QAbstractItemModel>

class SelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectionDialog(const QString &title, QAbstractItemModel *model, QWidget *parent = nullptr);
    ~SelectionDialog();

private:
    QAbstractItemModel *m_model;
    QTreeView *m_treeviewItems;
    QDialogButtonBox *m_buttonBox;
};

#endif // SELECTIONDIALOG_H
