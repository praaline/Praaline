#ifndef COMPAREANNOTATIONSWIDGET_H
#define COMPAREANNOTATIONSWIDGET_H

#include <QMainWindow>

namespace Ui {
class CompareAnnotationsWidget;
}

struct CompareAnnotationsWidgetData;

class CompareAnnotationsWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit CompareAnnotationsWidget(QWidget *parent = 0);
    ~CompareAnnotationsWidget();

private:
    Ui::CompareAnnotationsWidget *ui;
    CompareAnnotationsWidgetData *d;

    void setupActions();

private slots:
    void activeCorpusChanged(const QString &corpusID);
    void levelLeftChanged(QString text);
    void levelRightChanged(QString text);
    void annotationLeftChanged(QString text);
    void annotationRightChanged(QString text);
    void compare();
    void exportResults();
    void saveChanges();
};

#endif // COMPAREANNOTATIONSWIDGET_H
