#ifndef TABLEWIDGETBASE_H
#define TABLEWIDGETBASE_H

#include <QWidget>

namespace Ui {
class TableWidgetBase;
}

class TableWidgetBase : public QWidget
{
    Q_OBJECT

public:
    explicit TableWidgetBase(QWidget *parent = 0);
    ~TableWidgetBase();

private:
    Ui::TableWidgetBase *ui;
};

#endif // TABLEWIDGETBASE_H
