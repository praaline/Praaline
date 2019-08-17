#ifndef AUTOMATICANNOTATIONWIDGET_H
#define AUTOMATICANNOTATIONWIDGET_H

#include <QMainWindow>
#include "pncore/corpus/Corpus.h"
#include "app/interfaces/IAnnotationPlugin.h"
using namespace Praaline::Plugins;

namespace Ui {
class AutomaticAnnotationWidget;
}

struct AutomaticAnnotationWidgetData;

class AutomaticAnnotationWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit AutomaticAnnotationWidget(QWidget *parent = nullptr);
    ~AutomaticAnnotationWidget();

private slots:

    void actionAnnotate();


    void pluginSelectionChanged();

    void logAnnotationMessage(QString message);
    void pluginMadeProgress(int percentage);

private:
    Ui::AutomaticAnnotationWidget *ui;
    AutomaticAnnotationWidgetData *d;

    void setupActions();

    QList<CorpusCommunication *> selectedCommunications();
    QList<IAnnotationPlugin *> selectedPlugins();
};

#endif // AUTOMATICANNOTATIONWIDGET_H
