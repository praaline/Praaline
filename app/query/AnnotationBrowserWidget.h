#ifndef ANNOTATIONBROWSERWIDGET_H
#define ANNOTATIONBROWSERWIDGET_H

#include <QWidget>

struct AnnotationBrowserWidgetData;

class AnnotationBrowserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnnotationBrowserWidget(QWidget *parent = 0);
    ~AnnotationBrowserWidget();

signals:

public slots:

protected slots:
    void corpusAdded(QString);
    void corpusRemoved(QString);
    void selectedCorpusChanged(int index);
    void loadAnnotationData();

private:
    AnnotationBrowserWidgetData *d;
};

#endif // ANNOTATIONBROWSERWIDGET_H
