#ifndef IMPORTCORPUSITEMSWIZARDSELECTIONPAGE_H
#define IMPORTCORPUSITEMSWIZARDSELECTIONPAGE_H

#include <QWizardPage>
#include <QString>
#include <QPointer>
#include <QPair>
#include <QMap>
#include <QFileInfo>
#include <QStandardItemModel>

namespace Praaline {
namespace Core {
class CorpusRepository;
class CorpusRecording;
class CorpusAnnotation;
}
}

namespace Ui {
class ImportCorpusItemsWizardSelectionPage;
}

struct ImportCorpusItemsWizardSelectionPageData;

class ImportCorpusItemsWizardSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizardSelectionPage(Praaline::Core::CorpusRepository *repository,
                                                  QMap<QPair<QString, QString>, Praaline::Core::CorpusRecording *> &candidateRecordings,
                                                  QMap<QPair<QString, QString>, Praaline::Core::CorpusAnnotation *> &candidateAnnotations,
                                                  QWidget *parent = nullptr);
    ~ImportCorpusItemsWizardSelectionPage();

    bool validatePage ();

private slots:
    void selectFolder();
    void abortProcess();

private:
    Ui::ImportCorpusItemsWizardSelectionPage *ui;
    ImportCorpusItemsWizardSelectionPageData *d;

    QString communicationIDFromFilename(const QFileInfo &filename);
    QString speakerIDFromFilename(const QFileInfo &filename);
    void addFiles(const QString& directory, const QStringList &filters, bool recursive, bool isRecording);
    bool processFile(const QFileInfo &info, bool isRecording);
};

#endif // IMPORTCORPUSITEMSWIZARDSELECTIONPAGE_H
