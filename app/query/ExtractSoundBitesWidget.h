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
    void activeCorpusChanged(const QString &corpusID);
    void browseForFolder();
    void extractSoundBites();

private:
    Ui::ExtractSoundBitesWidget *ui;
    ExtractSoundBitesWidgetData *d;

    void prepareCopyOverCorpus(QPointer<Praaline::Core::Corpus> corpusSource, const QString &outputPath);
    void carryOverAnnotations(QPointer<Praaline::Core::Corpus> corpusSource,
                              QPointer<Praaline::Core::CorpusAnnotation> annotSource,
                              QPointer<Praaline::Core::CorpusAnnotation> annotDestination,
                              RealTime start, RealTime end);
};

#endif // EXTRACTSOUNDBITESWIDGET_H
