#ifndef IMPORTCORPUSITEMSWIZARDPROCESSMEDIAPAGE_H
#define IMPORTCORPUSITEMSWIZARDPROCESSMEDIAPAGE_H

#include <QWizardPage>
#include <QStandardItemModel>
#include "pncore/corpus/corpus.h"

namespace Ui {
class ImportCorpusItemsWizardProcessMediaPage;
}

class ImportCorpusItemsWizardProcessMediaPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizardProcessMediaPage(QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &candidateRecordings,
                                                     QWidget *parent = 0);
    ~ImportCorpusItemsWizardProcessMediaPage();

    void initializePage();
    bool validatePage();

private slots:
    void stopProcess();

private:
    Ui::ImportCorpusItemsWizardProcessMediaPage *ui;

    QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &m_candidateRecordings;
    QStandardItemModel *m_model;
    bool m_stop;

};

#endif // IMPORTCORPUSITEMSWIZARDPROCESSMEDIAPAGE_H
