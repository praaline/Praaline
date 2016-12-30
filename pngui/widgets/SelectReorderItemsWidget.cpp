#include "SelectReorderItemsWidget.h"

struct SelectReorderItemsWidgetData {
    int i;
};

SelectReorderItemsWidget::SelectReorderItemsWidget(QWidget *parent) :
    QWidget(parent), d(new SelectReorderItemsWidgetData)
{
}

SelectReorderItemsWidget::~SelectReorderItemsWidget()
{
    delete d;
}
