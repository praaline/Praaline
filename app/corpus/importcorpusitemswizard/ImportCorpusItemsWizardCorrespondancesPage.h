#ifndef IMPORTCORPUSITEMSWIZARDCORRESPONDANCESPAGE_H
#define IMPORTCORPUSITEMSWIZARDCORRESPONDANCESPAGE_H

#include <QWizardPage>
#include <QList>
#include <QSet>
#include <QMultiHash>
#include <QStandardItemModel>
#include "pncore/corpus/Corpus.h"
#include "ImportCorpusItemsWizardAnalysePage.h"
#include "ImportAnnotations.h"

namespace Ui {
class ImportCorpusItemsWizardCorrespondancesPage;
}

class ImportCorpusItemsWizardCorrespondancesPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizardCorrespondancesPage(QPointer<Corpus> corpus,
                                                        QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
                                                        QSet<QString> &tierNamesCommon,
                                                        QWidget *parent = 0);
    ~ImportCorpusItemsWizardCorrespondancesPage();
    void initializePage();
    bool validatePage();

private slots:
    void batchUpdate();

private:
    Ui::ImportCorpusItemsWizardCorrespondancesPage *ui;

    QPointer<Corpus> m_corpus;
    QMultiHash<QString, TierCorrespondance> &m_tierCorrespondances;
    QSet<QString> &m_tierNamesCommon;

    QStandardItemModel *m_modelTiers;

    void guessCorrespondance(int i, QString tierName);
    void guessCorrespondances();
};

#endif // IMPORTCORPUSITEMSWIZARDCORRESPONDANCESPAGE_H
