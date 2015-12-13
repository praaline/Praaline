#ifndef CORPUSITEMSELECTORWIDGET_H
#define CORPUSITEMSELECTORWIDGET_H

#include <QPointer>
#include <QWidget>
#include <QModelIndex>
#include "base/RealTime.h"

namespace Ui {
class CorpusItemSelectorWidget;
}

struct CorpusItemSelectorWidgetData;
class Corpus;
class CorpusCommunication;
class CorpusRecording;
class CorpusAnnotation;

class CorpusItemSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CorpusItemSelectorWidget(QWidget *parent = 0);
    ~CorpusItemSelectorWidget();

signals:
    void selectedCorpusCommunication(QPointer<Corpus>, QPointer<CorpusCommunication>);
    void selectedCorpusRecording(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusRecording>);
    void selectedCorpusAnnotation(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusAnnotation>);
    void moveToAnnotationTime(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusAnnotation>, RealTime);

private:
    Ui::CorpusItemSelectorWidget *ui;
    CorpusItemSelectorWidgetData *d;

    void setupActions();
    Corpus *findCorpus(QString corpusID);

private slots:
    void selectionChanged(QList<QObject*> selected);
    void openBookmarks();
    void saveBookmarks();
    void saveBookmarksAs();
    void bookmarkSelected(QModelIndex);

};

#endif // CORPUSITEMSELECTORWIDGET_H
