#ifndef IMPORTCORPUSITEMSWIZARDPROCESSMEDIAPAGE_H
#define IMPORTCORPUSITEMSWIZARDPROCESSMEDIAPAGE_H

#include <QWizardPage>
#include <QStandardItemModel>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class ImportCorpusItemsWizardProcessMediaPage;
}

class ImportCorpusItemsWizardProcessMediaPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizardProcessMediaPage(QMap<QPair<QString, QString>, CorpusRecording *> &candidateRecordings,
                                                     QWidget *parent = nullptr);
    ~ImportCorpusItemsWizardProcessMediaPage();

    void initializePage();
    bool validatePage();

private slots:
    void stopProcess();

private:
    Ui::ImportCorpusItemsWizardProcessMediaPage *ui;

    QMap<QPair<QString, QString>, CorpusRecording *> &m_candidateRecordings;
    QStandardItemModel *m_model;
    bool m_stop;

};

#endif // IMPORTCORPUSITEMSWIZARDPROCESSMEDIAPAGE_H
