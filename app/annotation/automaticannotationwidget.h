#ifndef AUTOMATICANNOTATIONWIDGET_H
#define AUTOMATICANNOTATIONWIDGET_H

#include <QMainWindow>
#include "pncore/corpus/corpus.h"
#include "iannotationplugin.h"
using namespace Praaline::Plugins;

namespace Ui {
class AutomaticAnnotationWidget;
}

struct AutomaticAnnotationWidgetData;

class AutomaticAnnotationWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit AutomaticAnnotationWidget(QWidget *parent = 0);
    ~AutomaticAnnotationWidget();

private slots:

    void actionAnnotate();
    void actionSaveOutput();
    void actionClearOutput();

    void pluginSelectionChanged();

    void logAnnotationMessage(QString message);
    void pluginMadeProgress(int percentage);

private:
    Ui::AutomaticAnnotationWidget *ui;
    AutomaticAnnotationWidgetData *d;

    void setupActions();

    QList<QPointer<CorpusCommunication> > selectedCommunications();
    QList<IAnnotationPlugin *> selectedPlugins();
};

#endif // AUTOMATICANNOTATIONWIDGET_H
