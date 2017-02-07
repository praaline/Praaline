#ifndef METADATAEDITORWIDGET_H
#define METADATAEDITORWIDGET_H

#include <QWidget>
#include <QString>
#include <QPointer>
#include <QtProperty>
#include <QPair>
#include <QMap>
#include "pncore/corpus/CorpusObject.h"
#include "pncore/structure/MetadataStructure.h"

struct MetadataEditorWidgetData;

class MetadataEditorWidget : public QWidget
{
    Q_OBJECT
public:
    class PropertyID {
    public:
        PropertyID() : type(Praaline::Core::CorpusObject::Type_Undefined), itemID(QString()), attributeID(QString()) {}
        PropertyID(Praaline::Core::CorpusObject::Type type, QString itemID, QString attributeID) :
            type(type), itemID(itemID), attributeID(attributeID) {}
        Praaline::Core::CorpusObject::Type type;
        QString itemID;
        QString attributeID;
    };

    enum MetadataEditorWidgetStyle {
        TreeStyle,
        GroupBoxStyle,
        ButtonStyle
    };

    MetadataEditorWidget(MetadataEditorWidgetStyle style = MetadataEditorWidget::TreeStyle, QWidget *parent = 0);
    ~MetadataEditorWidget();

    void setEditorStyle(MetadataEditorWidgetStyle style);
    void rebind(QPointer<Praaline::Core::MetadataStructure> mstructure,
                QList<QPointer<Praaline::Core::CorpusObject> > &items,
                bool includeParticipationSpeaker = false, bool includeParticipationCommunication = false);
    void clear();

signals:

public slots:

private slots:
    void valueChanged(QtProperty *property, const QString &value);
    void valueChanged(QtProperty *property, bool value);
    void valueChanged(QtProperty *property, int value);
    void valueChanged(QtProperty *property, double value);
    void valueChanged(QtProperty *property, const QDate &value);
    void valueChanged(QtProperty *property, const QTime &value);
    void valueChanged(QtProperty *property, const QDateTime &value);
    void enumValueChanged(QtProperty *property, const int index);

private:
    MetadataEditorWidgetData *d;

    void createPropertyBrowser(MetadataEditorWidgetStyle style);
    void addProperty(QtProperty *property, Praaline::Core::CorpusObject::Type type,
                     const QString &itemID, const QString &attributeID, QtProperty *group);
    QtProperty *addProperties(QPointer<Praaline::Core::MetadataStructure> mstructure,
                              QPointer<Praaline::Core::CorpusObject> item, QtProperty *under = 0);

};

inline bool operator<(const MetadataEditorWidget::PropertyID &id1, const MetadataEditorWidget::PropertyID &id2)
{
    if (id1.type != id2.type) return id1.type < id2.type;
    else if (id1.itemID != id2.itemID) return id1.itemID < id2.itemID;
    return id1.attributeID < id2.attributeID;
}

#endif // METADATAEDITORWIDGET_H
