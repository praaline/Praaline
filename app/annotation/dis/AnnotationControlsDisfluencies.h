#ifndef ANNOTATIONCONTROLSDISFLUENCIES_H
#define ANNOTATIONCONTROLSDISFLUENCIES_H

#include <QWidget>
#include "pngui/model/annotation/AnnotationTierModel.h"

namespace Ui {
class AnnotationControlsDisfluencies;
}

class AnnotationControlsDisfluencies : public QWidget
{
    Q_OBJECT

    enum DisfluencyTypesEnum {
        FIL, FST, LEN, WDP,
        REP, INS, SUB, DEL, COM
    };

public:
    explicit AnnotationControlsDisfluencies(QWidget *parent = 0);
    ~AnnotationControlsDisfluencies();

    void setModel(AnnotationTierModel *model, int columnToken, int columnDisfluency, int columnDiscourse);
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
    Ui::AnnotationControlsDisfluencies *ui;

    AnnotationTierModel *m_model;
    int m_columnToken, m_columnDisfluency, m_columnDiscourse;
    QList<int> m_rowsSelected;

    void setEnabledSimpleDisfluencies(bool enable);
    void setEnabledStructuredDisfluencies(bool enable);

    void annotateSimple(DisfluencyTypesEnum type);
    void annotateStructured(DisfluencyTypesEnum type);
};

#endif // ANNOTATIONCONTROLSDISFLUENCIES_H
