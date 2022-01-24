#ifndef IMPORTCORPUSITEMSWIZARDFINALPAGE_H
#define IMPORTCORPUSITEMSWIZARDFINALPAGE_H

#include <QWizardPage>
#include <QString>
#include <QMap>
#include <QSet>
#include <QPair>
#include <QMultiHash>

namespace Praaline {
namespace Core {
class CorpusRepository;
class CorpusCommunication;
class CorpusRecording;
class CorpusAnnotation;
class TierCorrespondance;
}
}


namespace Ui {
class ImportCorpusItemsWizardFinalPage;
}

struct ImportCorpusItemsWizardFinalPageData;

class ImportCorpusItemsWizardFinalPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizardFinalPage(Praaline::Core::CorpusRepository *repository,
                                              QMap<QPair<QString, QString>, Praaline::Core::CorpusRecording *> &candidateRecordings,
                                              QMap<QPair<QString, QString>, Praaline::Core::CorpusAnnotation *> &candidateAnnotations,
                                              QMultiHash<QString, Praaline::Core::TierCorrespondance> &tierCorrespondances,
                                              QSet<QString> &tierNamesCommon,
                                              QWidget *parent = nullptr);
    ~ImportCorpusItemsWizardFinalPage();

    void initializePage();
    bool validatePage();

private:
    Ui::ImportCorpusItemsWizardFinalPage *ui;
    ImportCorpusItemsWizardFinalPageData *d;

    void importPraat(Praaline::Core::CorpusCommunication *com, Praaline::Core::CorpusAnnotation *annot, QList<Praaline::Core::TierCorrespondance> &correspondances);
    void importTranscriber(Praaline::Core::CorpusCommunication *com, Praaline::Core::CorpusAnnotation *annot, QList<Praaline::Core::TierCorrespondance> &correspondances);
    void importSubRipTranscription(Praaline::Core::CorpusCommunication *com, Praaline::Core::CorpusAnnotation *annot, QList<Praaline::Core::TierCorrespondance> &correspondances);
    void importPhonTranscription(Praaline::Core::CorpusCommunication *com, Praaline::Core::CorpusAnnotation *annot, QList<Praaline::Core::TierCorrespondance> &correspondances);
};

#endif // IMPORTCORPUSITEMSWIZARDFINALPAGE_H
