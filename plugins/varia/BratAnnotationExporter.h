#ifndef BRATANNOTATIONEXPORTER_H
#define BRATANNOTATIONEXPORTER_H

#include <QString>
#include <QList>

class BratAnnotationExporter
{
public:
    class BratEntityType {
    public:
        QString type;
        QString label;
        QString labelAbbreviated;
        QString bgColour;
        QString borderColour;

        BratEntityType(const QString &type, const QString &label, const QString &labelAbbreviated,
                       const QString &bgColour, const QString borderColour) :
            type(type), label(label), labelAbbreviated(labelAbbreviated), bgColour(bgColour), borderColour(borderColour)
        {}
    };

    class BratRelationArgumentType {
    public:
        QString role;
        QStringList targets;
        QString colour;
        BratRelationArgumentType(const QString &role, const QStringList &targets, const QString &colour = QString()) :
            role(role), targets(targets), colour(colour)
        {}
    };

    class BratRelationType {
    public:
        QString type;
        QString label;
        QString labelAbbreviated;
        QString colour;
        QString dashArray;
        BratRelationArgumentType argumentFrom;
        BratRelationArgumentType argumentTo;
        BratRelationType(const QString &type, const QString &label, const QString &labelAbbreviated,
                         const QString &colour, const QString &dashArray,
                         const BratRelationArgumentType &argumentFrom, const BratRelationArgumentType &argumentTo) :
            type(type), label(label), labelAbbreviated(labelAbbreviated), colour(colour), dashArray(dashArray),
            argumentFrom(argumentFrom), argumentTo(argumentTo)
        {}
    };

    class BratEventArcType {
    public:
        QString type;
        QString label;
        QString labelAbbreviated;
        QString colour;
        BratEventArcType(const QString &type, const QString &label, const QString &labelAbbreviated, const QString &colour = QString()) :
            type(type), label(label), labelAbbreviated(labelAbbreviated), colour(colour)
        {}
    };

    class BratEventType {
    public:
        QString type;
        QString label;
        QString labelAbbreviated;
        QString bgColour;
        QString borderColour;
        QList<BratEventArcType> arcs;
        BratEventType(const QString &type, const QString &label, const QString &labelAbbreviated,
                      const QString &bgColour, const QString borderColour,
                      const QList<BratEventArcType> arcs = QList<BratEventArcType>()) :
            type(type), label(label), labelAbbreviated(labelAbbreviated), bgColour(bgColour), borderColour(borderColour),
            arcs(arcs)
        {}
    };

    class BratCollData {
    public:
        QList<BratEntityType> entityTypes;
        QList<BratRelationType> relationTypes;
        QList<BratEventType> eventTypes;
    };

    class BratEntity {
    public:
        QString ID;
        QString type;
        int start;
        int end;
        BratEntity(const QString &ID, const QString &type, int start, int end) :
            ID(ID), type(type), start(start), end(end)
        {}
    };

    class BratRelation {
    public:
        QString ID;
        QString type;
        QString argnameFrom;
        QString targetFrom;
        QString argnameTo;
        QString targetTo;
        BratRelation(const QString &ID, const QString &type,
                     const QString &argnameFrom, const QString &targetFrom, const QString &argnameTo, const QString &targetTo) :
            ID(ID), type(type),
            argnameFrom(argnameFrom), targetFrom(targetFrom), argnameTo(argnameTo), targetTo(targetTo)
        {}
    };

    class BratEventArc {
    public:
        QString arcType;
        QString arcID;
        BratEventArc(const QString &arcType, const QString &arcID) :
            arcType(arcType), arcID(arcID)
        {}
    };

    class BratEvent {
    public:
        QString ID;
        QString triggerID;
        QList<BratEventArc> arcs;
        BratEvent(const QString &ID, const QString &triggerID, const QList<BratEventArc> arcs = QList<BratEventArc>()) :
            ID(ID), triggerID(triggerID), arcs(arcs)
        {}
    };

    class BratDocData {
    public:
        QString divID;
        QString text;
        QList<BratEntity> entities;
        QList<BratRelation> relations;
        QList<BratEntity> triggers;
        QList<BratEvent> events;
    };


    BratAnnotationExporter();

    QString test();
    QString scriptCollData(const BratCollData &collData);
    QString scriptDocData(const BratDocData &docData);

private:
    QString scriptEntityTypes(const QList<BratEntityType> &entityTypes);
    QString scriptRelationTypes(const QList<BratRelationType> &entityTypes);
    QString scriptEventTypes(const QList<BratEventType> &entityTypes);
};

#endif // BRATANNOTATIONEXPORTER_H
