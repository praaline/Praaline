#ifndef TIMELINEEDITORCONFIGWIDGET_H
#define TIMELINEEDITORCONFIGWIDGET_H

#include <QWidget>
#include <QString>
#include <QList>
#include <QPair>
#include <QStandardItem>

namespace Ui {
class TimelineEditorConfigWidget;
}

struct TimelineEditorConfigWidgetData;

class TimelineEditorConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineEditorConfigWidget(QWidget *parent = 0);
    ~TimelineEditorConfigWidget();

    QList<QPair<QString, QString> > selectedLevelsAttributes() const;
    void updateSpeakerList(QStringList speakerIDs);

signals:
    void selectedLevelsAttributesChanged();
    void speakerAdded(const QString &speakerID);
    void speakerRemoved(const QString &speakerID);

protected slots:

    void addLevelAttribute();
    void removeLevelAttribute();
    void updateEditor();
    void saveConfiguration();
    void speakerSelectionChanged(QStandardItem *item);

private:
    Ui::TimelineEditorConfigWidget *ui;
    TimelineEditorConfigWidgetData *d;

    void setupActions();
    void addLevelAttribute(QString levelID, QString attributeID, QString function = QString(), QString parameters = QString());
};

#endif // TIMELINEEDITORCONFIGWIDGET_H
