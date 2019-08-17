#ifndef CONCORDANCERQUICKWIDGET_H
#define CONCORDANCERQUICKWIDGET_H

#include <QMainWindow>

namespace Ui {
class ConcordancerQuickWidget;
}

struct ConcordancerQuickWidgetData;

class ConcordancerQuickWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConcordancerQuickWidget(QWidget *parent = nullptr);
    ~ConcordancerQuickWidget();

private slots:
    void activeCorpusRepositoryChanged(const QString &repositoryID);
    void levelChanged(const QString &text);

    void definitionOpen();
    void definitionSave();
    void metadataFilterAdd();
    void metadataFilterRemove();
    void queryAddFilterGroup();
    void queryAddFilterSequence();
    void queryAddFilterAttribute();
    void queryRemove();
    void displayLevelsAttributesAdd();
    void displayLevelsAttributesRemove();
    void resultsSaveChanges();
    void resultsCreateBookmarks();

    void searchOccurrences();

private:
    Ui::ConcordancerQuickWidget *ui;
    ConcordancerQuickWidgetData *d;

    void setupActions();
};

#endif // CONCORDANCERQUICKWIDGET_H
