#ifndef IMPORTCORPUSITEMSWIZARDCORRESPONDANCESPAGE_H
#define IMPORTCORPUSITEMSWIZARDCORRESPONDANCESPAGE_H

#include <QWizardPage>
#include <QList>
#include <QSet>
#include <QMultiHash>
#include <QStandardItemModel>

#include "pncore/corpus/Corpus.h"
#include "pncore/interfaces/ImportAnnotations.h"
using namespace Praaline::Core;

#include "ImportCorpusItemsWizardAnalysePage.h"

namespace Ui {
class ImportCorpusItemsWizardCorrespondancesPage;
}

struct ImportCorpusItemsWizardCorrespondancesPageData;

class ImportCorpusItemsWizardCorrespondancesPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizardCorrespondancesPage(QPointer<Corpus> corpus,
                                                        QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
                                                        QSet<QString> &tierNamesCommon,
                                                        QWidget *parent = nullptr);
    ~ImportCorpusItemsWizardCorrespondancesPage();
    void initializePage();
    bool validatePage();

private slots:
    void batchUpdate();

private:
    Ui::ImportCorpusItemsWizardCorrespondancesPage *ui;
    ImportCorpusItemsWizardCorrespondancesPageData *d;

    void guessCorrespondance(int i, QString tierName);
    void guessCorrespondances();
};

#endif // IMPORTCORPUSITEMSWIZARDCORRESPONDANCESPAGE_H
