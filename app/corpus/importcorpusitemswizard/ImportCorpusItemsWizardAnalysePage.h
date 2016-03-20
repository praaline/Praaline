#ifndef IMPORTCORPUSITEMSWIZARDANALYSEPAGE_H
#define IMPORTCORPUSITEMSWIZARDANALYSEPAGE_H

#include <QWizardPage>
#include <QStringList>
#include <QSet>
#include <QMultiHash>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include "ImportAnnotations.h"
#include "pncore/corpus/corpus.h"

namespace Ui {
class ImportCorpusItemsWizardAnalysePage;
}

class ImportCorpusItemsDelegateSpeakerPolicy : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ImportCorpusItemsDelegateSpeakerPolicy(QObject *parent = 0) : QStyledItemDelegate(parent) {}
    ~ImportCorpusItemsDelegateSpeakerPolicy() {}
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

class ImportCorpusItemsWizardAnalysePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizardAnalysePage(QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &candidateAnnotations,
                                                QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
                                                QSet<QString> &tierNamesCommon,
                                                QWidget *parent = 0);
    ~ImportCorpusItemsWizardAnalysePage();

    void initializePage();
    bool validatePage();

private slots:
    void stopProcess();
    void updateSpeakerIDTier();
    void copyAnalysisTable();

private:
    Ui::ImportCorpusItemsWizardAnalysePage *ui;
    // State
    QStandardItemModel *m_modelAnnotations;
    QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &m_candidateAnnotations;
    QMultiHash<QString, TierCorrespondance> &m_tierCorrespondances;
    QSet<QString> &m_tierNamesCommon;
    bool m_stop;

    bool loadTextgrids();

};

#endif // IMPORTCORPUSITEMSWIZARDANALYSEPAGE_H
