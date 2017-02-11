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

    void setTranscriptionLevelID(const QString &levelID);
    QString transcriptionLevelID() const;

    void setSkipPauses(bool skip);
    bool skipPauses() const;

    void setAnnotation(QPointer<Praaline::Core::CorpusAnnotation> annot);
    void clear();

private:
    MiniTranscriptionWidgetData *d;
    void rebind(QPointer<Praaline::Core::CorpusAnnotation> annot, const QString &levelID);
    void asyncCreateTranscript(QPointer<Praaline::Core::CorpusAnnotation> annot);

private slots:
    void asyncCreateTranscriptFinished();
};

#endif // MINITRANSCRIPTIONWIDGET_H
