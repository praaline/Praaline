#ifndef LANGUAGEMODELBUILDERWIDGET_H
#define LANGUAGEMODELBUILDERWIDGET_H

#include <QWidget>

namespace Ui {
class LanguageModelBuilderWidget;
}

class LanguageModelBuilderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LanguageModelBuilderWidget(QWidget *parent = 0);
    ~LanguageModelBuilderWidget();

private:
    Ui::LanguageModelBuilderWidget *ui;
};

#endif // LANGUAGEMODELBUILDERWIDGET_H
