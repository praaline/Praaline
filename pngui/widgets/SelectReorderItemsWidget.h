#ifndef SELECTREORDERITEMSWIDGET_H
#define SELECTREORDERITEMSWIDGET_H

#include <QWidget>

struct SelectReorderItemsWidgetData;

class SelectReorderItemsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SelectReorderItemsWidget(QWidget *parent = 0);
    ~SelectReorderItemsWidget();

signals:

public slots:

private:
    SelectReorderItemsWidgetData *d;
};

#endif // SELECTREORDERITEMSWIDGET_H