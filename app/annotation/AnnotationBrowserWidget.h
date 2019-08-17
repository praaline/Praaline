#ifndef ANNOTATIONBROWSERWIDGET_H
#define ANNOTATIONBROWSERWIDGET_H

#include <QWidget>

struct AnnotationBrowserWidgetData;

class AnnotationBrowserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnnotationBrowserWidget(QWidget *parent = nullptr);
    ~AnnotationBrowserWidget();

signals:

public slots:

protected slots:
    void corpusRepositoryAdded(const QString &repositoryID);
    void corpusRepositoryRemoved(const QString &repositoryID);
    void selectedCorpusRepositoryChanged(int index);
    void loadAnnotationData();
    void saveAnnotationData();

private:
    AnnotationBrowserWidgetData *d;
    void setupActions();
};

#endif // ANNOTATIONBROWSERWIDGET_H
