#ifndef CORPUSCOMMUNICATIONSPEAKERRELATIONSWIDGET_H
#define CORPUSCOMMUNICATIONSPEAKERRELATIONSWIDGET_H

#include <QWidget>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

namespace Ui {
class CorpusCommunicationSpeakerRelationsWidget;
}

struct CorpusCommunicationSpeakerRelationsWidgetData;

class CorpusCommunicationSpeakerRelationsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CorpusCommunicationSpeakerRelationsWidget(QWidget *parent = nullptr);
    ~CorpusCommunicationSpeakerRelationsWidget();

    void openCommunication(Praaline::Core::CorpusCommunication *com);
    void clear();

private slots:
    void addSpeakerRelation();
    void removeSpeakerRelation();

private:
    Ui::CorpusCommunicationSpeakerRelationsWidget *ui;
    CorpusCommunicationSpeakerRelationsWidgetData *d;

    void setupActions();
};

#endif // CORPUSCOMMUNICATIONSPEAKERRELATIONSWIDGET_H
