#ifndef GRIDVIEWFINDDIALOG_H
#define GRIDVIEWFINDDIALOG_H

#include <QDialog>

namespace Ui {
class GridViewFindDialog;
}

class GridViewFindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GridViewFindDialog(QWidget *parent = 0);
    ~GridViewFindDialog();

    enum SearchScope {
        EntireTable,
        Rows,
        Columns
    };

    QString findWhat() const;
    SearchScope searchScope() const;
    void setDefaultSearchScope(SearchScope);

    bool matchWholeWordsOnly() const;
    bool matchCase() const;
    bool useRegularExpressions() const;
    bool directionIsDown() const;
    bool wrapAround() const;

signals:
    void findNext();

protected slots:
    void findNextClicked();

private:
    Ui::GridViewFindDialog *ui;
};

#endif // GRIDVIEWFINDDIALOG_H
