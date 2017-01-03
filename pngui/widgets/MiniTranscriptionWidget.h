#ifndef MINITRANSCRIPTIONWIDGET_H
#define MINITRANSCRIPTIONWIDGET_H

#include <QWidget>

struct CorpusItemPreviewData;
namespace Praaline {
namespace Core {
class CorpusAnnotation;
}
}

struct MiniTranscriptionWidgetData;

class MiniTranscriptionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MiniTranscriptionWidget(QWidget *parent = 0);
    virtual ~MiniTranscriptionWidget();

    void setAnnotation(QPointer<Praaline::Core::CorpusAnnotation> annot);
    void setTranscriptionLevelID(const QString &levelID);

signals:

public slots:

private:
    MiniTranscriptionWidgetData *d;
};

#endif // MINITRANSCRIPTIONWIDGET_H
