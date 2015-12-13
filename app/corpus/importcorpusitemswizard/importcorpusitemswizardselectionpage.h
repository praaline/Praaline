#ifndef IMPORTCORPUSITEMSWIZARDSELECTIONPAGE_H
#define IMPORTCORPUSITEMSWIZARDSELECTIONPAGE_H

#include <QWizardPage>
#include <QString>
#include <QPointer>
#include <QPair>
#include <QMap>
#include <QFileInfo>
#include <QStandardItemModel>
#include "pncore/corpus/corpus.h"
#include "importannotations.h"

namespace Ui {
class ImportCorpusItemsWizardSelectionPage;
}

class ImportCorpusItemsWizardSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizardSelectionPage(QPointer<Corpus> corpus,
                                                  QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &candidateRecordings,
                                                  QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &candidateAnnotations,
                                                  QWidget *parent = 0);
    ~ImportCorpusItemsWizardSelectionPage();

    bool validatePage ();

private slots:
    void selectFolder();
    void abortProcess();

private:
    Ui::ImportCorpusItemsWizardSelectionPage *ui;
    // State
    QPointer<Corpus> m_corpus;
    QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &m_candidateRecordings;
    QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &m_candidateAnnotations;
    QPointer<QStandardItemModel> m_modelFormatsRecording;
    QPointer<QStandardItemModel> m_modelFormatsAnnotation;
    bool m_abort;
    // Methods
    QString communicationNameFromFilename(const QFileInfo &filename);
    void addFiles(const QString& directory, const QStringList &filters, bool recursive, bool isRecording);
    bool processFile(const QFileInfo &info, bool isRecording);
};

#endif // IMPORTCORPUSITEMSWIZARDSELECTIONPAGE_H
