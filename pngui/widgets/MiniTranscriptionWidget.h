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
    explicit MiniTranscriptionWidget(QWidget *parent = nullptr);
    virtual ~MiniTranscriptionWidget();

    void setTranscriptionLevelID(const QString &levelID);
    QString transcriptionLevelID() const;

    void setSkipPauses(bool skip);
    bool skipPauses() const;

    void setAnnotation(Praaline::Core::CorpusAnnotation *annot);
    void clear();

private:
    MiniTranscriptionWidgetData *d;
    void rebind(Praaline::Core::CorpusAnnotation *annot, const QString &levelID);
    void asyncCreateTranscript(Praaline::Core::CorpusAnnotation *annot);

private slots:
    void asyncCreateTranscriptFinished();
};

#endif // MINITRANSCRIPTIONWIDGET_H
