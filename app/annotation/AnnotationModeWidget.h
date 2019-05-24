#ifndef ANNOTATIONMODEWIDGET_H
#define ANNOTATIONMODEWIDGET_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QTextStream>
#include "pngui/observers/CorpusObserver.h"

namespace Ui {
class AnnotationModeWidget;
}

struct AnnotationModeWidgetData;

class AnnotationModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnnotationModeWidget(QWidget *parent = nullptr);
    ~AnnotationModeWidget();

signals:
    void activateMode();

private:
    Ui::AnnotationModeWidget *ui;
    AnnotationModeWidgetData *d;

    void setupActions();

private slots:
    void showAutomaticAnnotation();
    void showManualAnnotation();
    void showTranscriber();
    void showSpeechRecognition();
    void showAnnotationBrowser();
    void showBatchEditor();
    void showCompareAnnotations();

    void dialogAddCalculatedAnnotation();
    void dialogCreateSequenceAnnotation();
    void dialogComposeTranscription();
    void dialogTidyUpAnnotations();
};

#endif // ANNOTATIONMODEWIDGET_H
