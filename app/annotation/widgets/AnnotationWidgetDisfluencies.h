#ifndef ANNOTATIONWIDGETDISFLUENCIES_H
#define ANNOTATIONWIDGETDISFLUENCIES_H

#include <QWidget>
#include "pngui/model/annotation/AnnotationMultiTierTableModel.h"

namespace Ui {
class AnnotationWidgetDisfluencies;
}

struct AnnotationWidgetDisfluenciesData;

class AnnotationWidgetDisfluencies : public QWidget
{
    Q_OBJECT

    enum DisfluencyTypesEnum {
        FIL, FST, LEN, WDP,
        REP, INS, SUB, DEL, COM
    };

public:
    explicit AnnotationWidgetDisfluencies(QWidget *parent = nullptr);
    ~AnnotationWidgetDisfluencies();

    void setModel(AnnotationMultiTierTableModel *model);
    void setColumnIndexes(int columnIndexToken, int columnIndexDisfluency, int columnIndexDiscourse);
    void setSelection(QList<int> rowsSelected);

private slots:
    void on_cmdFilledPause_clicked();
    void on_cmdFalseStart_clicked();
    void on_cmdLengthening_clicked();
    void on_cmdWithinWordPause_clicked();
    void on_cmdRepetition_clicked();
    void on_cmdSubstitution_clicked();
    void on_cmdInsertion_clicked();
    void on_cmdDeletion_clicked();
    void on_cmdComplex_clicked();
    // boundaries
    void on_cmdBoundaryBreak_clicked();
    void on_cmdBoundaryContinue_clicked();

    void on_cmdDiscourseMarker_clicked();

private:
    Ui::AnnotationWidgetDisfluencies *ui;
    AnnotationWidgetDisfluenciesData *d;

    void setEnabledSimpleDisfluencies(bool enable);
    void setEnabledStructuredDisfluencies(bool enable);

    void annotateSimple(DisfluencyTypesEnum type);
    void annotateStructured(DisfluencyTypesEnum type);
};

#endif // ANNOTATIONWIDGETDISFLUENCIES_H
