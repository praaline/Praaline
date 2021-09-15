#ifndef ANNOTATIONWIDGETSEQUENCES_H
#define ANNOTATIONWIDGETSEQUENCES_H

#include <QWidget>
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "pngui/model/annotation/AnnotationMultiTierTableModel.h"

namespace Ui {
class AnnotationWidgetSequences;
}

struct AnnotationWidgetSequencesData;

class AnnotationWidgetSequences : public QWidget
{
    Q_OBJECT

public:
    explicit AnnotationWidgetSequences(QWidget *parent = nullptr);
    ~AnnotationWidgetSequences();

    void setCorpusRepositoryAndModel(Praaline::Core::CorpusRepository *repository, AnnotationMultiTierTableModel *model);
    void setSelection(QList<int> rowsSelected);

private slots:
    void annotationLevelChanged(int index);
    void on_cmdAddSequence_clicked();
    void on_cmdDeleteSequence_clicked();

private:
    Ui::AnnotationWidgetSequences *ui;
    AnnotationWidgetSequencesData *d;
};

#endif // ANNOTATIONWIDGETSEQUENCES_H
