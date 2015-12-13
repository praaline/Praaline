#include <QDialog>
#include <QDialogButtonBox>
#include <QTreeView>
#include <QVBoxLayout>
#include "selectiondialog.h"

SelectionDialog::SelectionDialog(const QString &title, QAbstractItemModel *model, QWidget *parent) :
    QDialog(parent), m_model(model)
{
    m_treeviewItems = new QTreeView(this);
    m_treeviewItems->setModel(model);
    m_treeviewItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int i = 0; i < m_model->columnCount(); ++i)
        m_treeviewItems->resizeColumnToContents(i);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_treeviewItems);
    mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);

    setWindowTitle(title);
}

SelectionDialog::~SelectionDialog()
{
}

