#ifndef CHECKBOXLIST_H
#define CHECKBOXLIST_H

#include <QObject>
#include <QString>
#include <QWidget>
#include <QComboBox>
#include <QPaintEvent>

class CheckBoxList: public QComboBox
{
    Q_OBJECT

public:
    CheckBoxList(QWidget *widget = 0);
    virtual ~CheckBoxList();

    bool eventFilter(QObject *object, QEvent *event);
    virtual void paintEvent(QPaintEvent *);
    void setDisplayText(QString text);
    QString getDisplayText() const;

private:
    QString m_DisplayText;
};

#endif // CHECKBOXLIST_H
