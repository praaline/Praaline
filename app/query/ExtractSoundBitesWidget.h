#ifndef EXTRACTSOUNDBITESWIDGET_H
#define EXTRACTSOUNDBITESWIDGET_H

#include <QPointer>
#include <QString>
#include <QMainWindow>
#include "base/RealTime.h"

namespace Ui {
class ExtractSoundBitesWidget;
}

namespace Praaline {
namespace Core {
class CorpusRepository;
class Corpus;
class CorpusAnnotation;
}
}

struct ExtractSoundBitesWidgetData;

class ExtractSoundBitesWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit ExtractSoundBitesWidget(QWidget *parent = 0);
    ~ExtractSoundBitesWidget();

private slots:
    void activeCorpusRepositoryChanged(const QString &repositoryID);
    void browseForFolder();
    void manualEntryClearTable();
    void extractSoundBites();

private:
    Ui::ExtractSoundBitesWidget *ui;
    ExtractSoundBitesWidgetData *d;

    void prepareCopyOverCorpus(QPointer<Praaline::Core::CorpusRepository> repositorySource, const QString &outputPath);
    void carryOverAnnotations(QPointer<Praaline::Core::CorpusRepository> repositorySource,
                              QPointer<Praaline::Core::CorpusAnnotation> annotSource,
                              QPointer<Praaline::Core::CorpusAnnotation> annotDestination,
                              RealTime start, RealTime end);
};

#endif // EXTRACTSOUNDBITESWIDGET_H
