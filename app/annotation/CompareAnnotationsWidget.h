#ifndef COMPAREANNOTATIONSWIDGET_H
#define COMPAREANNOTATIONSWIDGET_H

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QPointer>

namespace Ui {
class CompareAnnotationsWidget;
}

class DiffSESforIntervalsTableModel;
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
    // Repositories
    void corpusRepositoryAdded(const QString &);
    void corpusRepositoryRemoved(const QString &);
    // Compare Corpora
    void ccRepositoryLeftChanged(const QString &);
    void ccRepositoryRightChanged(const QString &);
    // Compare Annotations
    void caRepositoryLeftChanged(const QString &);
    void caRepositoryRightChanged(const QString &);
    void caCorpusLeftChanged(const QString &);
    void caCorpusRightChanged(const QString &);
    void caLevelLeftChanged(const QString &);
    void caLevelRightChanged(const QString &);
    void caAnnotationLeftChanged(const QString &);
    void caAnnotationRightChanged(const QString &);

    // Commands
    void compareAnnotations();
    void compareCorpora();
    void exportResults();
    void saveChanges();

    // Export
    void exportDiffTableByLevelExcel(const QString &communicationID, const QString &annotationID, const QString &speakerID,
                                     QMap<QString, QPointer<DiffSESforIntervalsTableModel> > &models);
    void exportDiffTableCombinedExcel(const QString &communicationID, const QString &annotationID, const QString &speakerID,
                                      QMap<QString, QPointer<DiffSESforIntervalsTableModel> > &models);

};

#endif // COMPAREANNOTATIONSWIDGET_H
