#ifndef CORPUSITEMSELECTORWIDGET_H
#define CORPUSITEMSELECTORWIDGET_H

#include <QPointer>
#include <QWidget>
#include <QModelIndex>
#include "PraalineCore/Base/RealTime.h"

namespace Ui {
class CorpusItemSelectorWidget;
}

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class CorpusRecording;
class CorpusAnnotation;
}
}
using namespace Praaline::Core;

namespace Qtilities {
namespace Core {
class Observer;
}
}

struct CorpusItemSelectorWidgetData;


class CorpusItemSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CorpusItemSelectorWidget(QWidget *parent = nullptr);
    ~CorpusItemSelectorWidget();

public slots:
    void addBookmark(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                     const RealTime &timeStart, const RealTime &timeEnd, const QString &name);

signals:
    void selectedCorpusCommunication(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>);
    void selectedCorpusRecording(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>, QPointer<Praaline::Core::CorpusRecording>);
    void selectedCorpusAnnotation(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>, QPointer<Praaline::Core::CorpusAnnotation>);
    void moveToAnnotationTime(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>, QPointer<Praaline::Core::CorpusAnnotation>, RealTime);

private:
    Ui::CorpusItemSelectorWidget *ui;
    CorpusItemSelectorWidgetData *d;

    void setupActions();
    Praaline::Core::Corpus *findCorpus(QString corpusID);

private slots:
    void doubleClickRequest(QObject* object, Qtilities::Core::Observer* parent_observer);
    void selectionChanged(QList<QObject*> selected);
    void openBookmarks();
    void saveBookmarks();
    void saveBookmarksAs();
    void bookmarkSelected(QModelIndex);

};

#endif // CORPUSITEMSELECTORWIDGET_H
