#ifndef ADDATTRIBUTEDIALOG_H
#define ADDATTRIBUTEDIALOG_H

#include <QDialog>

namespace Ui {
class AddAttributeDialog;
}

class AddAttributeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddAttributeDialog(QWidget *parent = 0);
    ~AddAttributeDialog();

    QString attributeID() const;
    QString datatype() const;
    int datalength() const;

private slots:
    void datatypeChanged(QString text);

private:
    Ui::AddAttributeDialog *ui;
};

#endif // ADDATTRIBUTEDIALOG_H
