#ifndef ADDLEVELDIALOG_H
#define ADDLEVELDIALOG_H

#include <QDialog>
#include "PraalineCore/Structure/AnnotationStructure.h"
#include "PraalineCore/Structure/AnnotationStructureLevel.h"
using namespace Praaline::Core;

namespace Ui {
class AddLevelDialog;
}

class AddLevelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddLevelDialog(AnnotationStructure *structure, QWidget *parent = nullptr);
    ~AddLevelDialog();

    QString levelID() const;
    AnnotationStructureLevel::LevelType levelType() const;
    QString parentLevelID() const;
    QString datatype() const;
    int datalength() const;

private slots:
    void levelTypeChanged(QString text);
    void datatypeChanged(QString text);

private:
    Ui::AddLevelDialog *ui;
    AnnotationStructure *m_annotationStructure;
};

#endif // ADDLEVELDIALOG_H
